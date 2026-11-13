require("dotenv").config();

const express = require("express");
const cors = require("cors");
const fs = require("fs");
const path = require("path");

const { config, rootDir } = require("./config");
const { closePool } = require("./db");
const {
  getRun,
  getRunEvents,
  getRunMetrics,
  getRunTrajectory,
  listRuns
} = require("./services/runRepository");
const {
  cancelRun,
  launchRun,
  parseRunRequest,
  subscribeToRun
} = require("./services/runManager");

const app = express();
app.use(cors());
app.use(express.json({ limit: "1mb" }));

app.get("/api/health", (_req, res) => {
  res.json({
    status: "ok",
    service: "vislam-api",
    mlConfidenceEnabled: config.enableMlConfidence
  });
});

function resolveDatasetPath(rawPath) {
  return path.isAbsolute(rawPath) ? rawPath : path.resolve(rootDir, rawPath);
}

function findEurocRoot(candidatePath) {
  const direct = path.join(candidatePath, "mav0");
  if (fs.existsSync(direct)) {
    return candidatePath;
  }

  const entries = fs.existsSync(candidatePath)
    ? fs.readdirSync(candidatePath, { withFileTypes: true })
    : [];

  for (const first of entries) {
    if (!first.isDirectory()) continue;
    const firstPath = path.join(candidatePath, first.name);
    if (fs.existsSync(path.join(firstPath, "mav0"))) {
      return firstPath;
    }

    const nested = fs.readdirSync(firstPath, { withFileTypes: true });
    for (const second of nested) {
      if (!second.isDirectory()) continue;
      const secondPath = path.join(firstPath, second.name);
      if (fs.existsSync(path.join(secondPath, "mav0"))) {
        return secondPath;
      }
    }
  }

  return candidatePath;
}

app.get("/api/datasets/euroc/validate", (req, res) => {
  const rawPath = String(req.query.path || "").trim();
  if (!rawPath) {
    return res.status(400).json({
      isValid: false,
      reason: "Missing query parameter: path"
    });
  }

  const resolvedInputPath = resolveDatasetPath(rawPath);
  if (!fs.existsSync(resolvedInputPath)) {
    return res.json({
      isValid: false,
      reason: "Provided path does not exist",
      resolvedInputPath
    });
  }

  const eurocRoot = findEurocRoot(resolvedInputPath);
  const camCsv = path.join(eurocRoot, "mav0", "cam0", "data.csv");
  const imuCsv = path.join(eurocRoot, "mav0", "imu0", "data.csv");
  const hasCamCsv = fs.existsSync(camCsv);
  const hasImuCsv = fs.existsSync(imuCsv);
  const isValid = hasCamCsv && hasImuCsv;

  return res.json({
    isValid,
    reason: isValid
      ? "EuRoC layout found"
      : "Expected mav0/cam0/data.csv and mav0/imu0/data.csv",
    resolvedInputPath,
    detectedEurocRoot: eurocRoot,
    checks: {
      camCsv,
      imuCsv,
      hasCamCsv,
      hasImuCsv
    }
  });
});

app.post("/api/runs", async (req, res, next) => {
  try {
    const parsed = parseRunRequest(req.body);
    if (!parsed.ok) {
      return res.status(400).json({
        error: "Invalid run payload",
        details: parsed.error
      });
    }
    const run = await launchRun(parsed.value);
    return res.status(202).json({
      runId: run.id,
      status: run.status
    });
  } catch (error) {
    return next(error);
  }
});

app.get("/api/runs", async (req, res, next) => {
  try {
    const limit = Number(req.query.limit || 20);
    const runs = await listRuns(Number.isNaN(limit) ? 20 : Math.min(limit, 100));
    res.json({ runs });
  } catch (error) {
    next(error);
  }
});

app.get("/api/runs/:id", async (req, res, next) => {
  try {
    const run = await getRun(req.params.id);
    if (!run) {
      return res.status(404).json({ error: "Run not found" });
    }
    return res.json(run);
  } catch (error) {
    return next(error);
  }
});

app.get("/api/runs/:id/trajectory", async (req, res, next) => {
  try {
    const run = await getRun(req.params.id);
    if (!run) return res.status(404).json({ error: "Run not found" });
    const poses = await getRunTrajectory(req.params.id);
    return res.json({ runId: req.params.id, poses });
  } catch (error) {
    return next(error);
  }
});

app.get("/api/runs/:id/metrics", async (req, res, next) => {
  try {
    const run = await getRun(req.params.id);
    if (!run) return res.status(404).json({ error: "Run not found" });
    const metrics = await getRunMetrics(req.params.id);
    return res.json({ runId: req.params.id, metrics });
  } catch (error) {
    return next(error);
  }
});

app.get("/api/runs/:id/logs", async (req, res, next) => {
  try {
    const run = await getRun(req.params.id);
    if (!run) return res.status(404).json({ error: "Run not found" });
    const events = await getRunEvents(req.params.id);
    return res.json({ runId: req.params.id, events });
  } catch (error) {
    return next(error);
  }
});

app.get("/api/runs/:id/stream", async (req, res, next) => {
  try {
    const run = await getRun(req.params.id);
    if (!run) return res.status(404).json({ error: "Run not found" });

    res.writeHead(200, {
      "Content-Type": "text/event-stream",
      "Cache-Control": "no-cache",
      Connection: "keep-alive"
    });

    res.write(`event: status\n`);
    res.write(`data: ${JSON.stringify({ status: run.status })}\n\n`);

    const unsubscribe = subscribeToRun(req.params.id, (event) => {
      res.write(`event: ${event.type}\n`);
      res.write(`data: ${JSON.stringify(event)}\n\n`);
    });

    req.on("close", () => {
      unsubscribe();
      res.end();
    });
  } catch (error) {
    next(error);
  }
});

app.post("/api/runs/:id/cancel", async (req, res, next) => {
  try {
    const run = await getRun(req.params.id);
    if (!run) return res.status(404).json({ error: "Run not found" });
    const cancelled = await cancelRun(req.params.id);
    return res.json({ runId: req.params.id, cancelled });
  } catch (error) {
    return next(error);
  }
});

app.use((err, _req, res, _next) => {
  console.error(err);
  res.status(500).json({
    error: "Internal server error",
    message: err.message
  });
});

const server = app.listen(config.port, () => {
  console.log(`vislam-api listening on port ${config.port}`);
});

async function shutdown(signal) {
  console.log(`Received ${signal}. Shutting down.`);
  server.close(async () => {
    await closePool();
    process.exit(0);
  });
}

process.on("SIGINT", () => void shutdown("SIGINT"));
process.on("SIGTERM", () => void shutdown("SIGTERM"));
