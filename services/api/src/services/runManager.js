const fs = require("fs");
const path = require("path");
const { spawn } = require("child_process");
const { EventEmitter } = require("events");
const { v4: uuidv4 } = require("uuid");
const { z } = require("zod");

const { config, rootDir } = require("../config");
const {
  addRunEvent,
  createRun,
  replaceRunMetrics,
  replaceRunTrajectory,
  updateRunStatus
} = require("./runRepository");
const { readTrajectoryFile } = require("./trajectory");
const { buildMetrics } = require("./metrics");
const { loadModel } = require("../ml/confidence");

const runRequestSchema = z.object({
  dataset: z.string().trim().min(1).default("euroc"),
  datasetPath: z.string().trim().min(1),
  maxFrames: z.number().int().positive().max(50000).default(300)
});

const activeRuns = new Map();
const runBus = new EventEmitter();
runBus.setMaxListeners(200);

function buildOutputPath(runId) {
  return path.join(config.artifactsDir, runId, "trajectory.txt");
}

function emitRunEvent(runId, type, payload) {
  runBus.emit(`run:${runId}`, { type, payload, timestamp: new Date().toISOString() });
}

async function appendEvent(runId, level, message) {
  await addRunEvent(runId, level, message);
  emitRunEvent(runId, "log", { level, message });
}

function ensureArtifactsDirectory(runId) {
  const runDir = path.join(config.artifactsDir, runId);
  fs.mkdirSync(runDir, { recursive: true });
  return runDir;
}

function parseRunRequest(body) {
  const parsed = runRequestSchema.safeParse(body);
  if (!parsed.success) {
    return { ok: false, error: parsed.error.flatten() };
  }
  return { ok: true, value: parsed.data };
}

async function launchRun({ dataset, datasetPath, maxFrames }) {
  const runId = uuidv4();
  ensureArtifactsDirectory(runId);
  const outputPath = buildOutputPath(runId);

  const run = {
    id: runId,
    dataset,
    datasetPath,
    maxFrames,
    outputPath,
    status: "queued"
  };

  await createRun(run);
  emitRunEvent(runId, "status", { status: "queued" });

  void executeRun(run).catch(async (error) => {
    await updateRunStatus(runId, "failed", {
      errorMessage: error.message,
      completedAtNow: true
    });
    await appendEvent(runId, "error", `Unhandled run error: ${error.message}`);
    emitRunEvent(runId, "status", { status: "failed", errorMessage: error.message });
    activeRuns.delete(runId);
  });

  return run;
}

async function executeRun(run) {
  if (!fs.existsSync(config.vislamExecutable)) {
    throw new Error(
      `vislam_app executable not found at ${config.vislamExecutable}. Set VISLAM_EXECUTABLE.`
    );
  }

  const args = [
    "--dataset",
    run.dataset,
    "--path",
    run.datasetPath,
    "--output",
    run.outputPath,
    "--max-frames",
    String(run.maxFrames)
  ];

  const child = spawn(config.vislamExecutable, args, {
    cwd: rootDir,
    stdio: ["ignore", "pipe", "pipe"]
  });
  activeRuns.set(run.id, child);

  await updateRunStatus(run.id, "running", { startedAtNow: true });
  await appendEvent(run.id, "info", `Started vislam_app with PID ${child.pid}`);
  emitRunEvent(run.id, "status", { status: "running" });

  child.stdout.on("data", (buf) => {
    const message = buf.toString("utf8").trim();
    if (!message) return;
    void appendEvent(run.id, "stdout", message);
  });
  child.stderr.on("data", (buf) => {
    const message = buf.toString("utf8").trim();
    if (!message) return;
    void appendEvent(run.id, "stderr", message);
  });

  const exitCode = await new Promise((resolve) => {
    child.on("close", resolve);
  });
  activeRuns.delete(run.id);

  if (exitCode !== 0) {
    await updateRunStatus(run.id, "failed", {
      errorMessage: `vislam_app exited with code ${exitCode}`,
      exitCode,
      completedAtNow: true
    });
    await appendEvent(run.id, "error", `Run failed with exit code ${exitCode}`);
    emitRunEvent(run.id, "status", {
      status: "failed",
      exitCode
    });
    return;
  }

  await appendEvent(run.id, "info", "Run completed. Parsing trajectory.");
  const poses = readTrajectoryFile(run.outputPath);
  await replaceRunTrajectory(run.id, poses);

  let model = null;
  if (config.enableMlConfidence && process.env.ML_CONFIDENCE_MODEL_PATH) {
    model = loadModel(process.env.ML_CONFIDENCE_MODEL_PATH);
  }
  const metrics = buildMetrics(poses, {
    model,
    mlEnabled: config.enableMlConfidence
  });
  await replaceRunMetrics(run.id, metrics);

  await updateRunStatus(run.id, "completed", {
    exitCode: 0,
    completedAtNow: true
  });
  await appendEvent(
    run.id,
    "info",
    `Stored ${poses.length} poses and ${metrics.length} metrics.`
  );
  emitRunEvent(run.id, "status", { status: "completed", exitCode: 0 });
}

async function cancelRun(runId) {
  const child = activeRuns.get(runId);
  if (!child) {
    return false;
  }
  child.kill("SIGTERM");
  await appendEvent(runId, "info", "Cancellation requested.");
  await updateRunStatus(runId, "cancelled", { completedAtNow: true });
  emitRunEvent(runId, "status", { status: "cancelled" });
  activeRuns.delete(runId);
  return true;
}

function subscribeToRun(runId, listener) {
  const key = `run:${runId}`;
  runBus.on(key, listener);
  return () => {
    runBus.off(key, listener);
  };
}

module.exports = {
  cancelRun,
  launchRun,
  parseRunRequest,
  subscribeToRun
};

