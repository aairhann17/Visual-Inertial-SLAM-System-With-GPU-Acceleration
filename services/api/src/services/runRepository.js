const { query } = require("../db");

async function createRun(run) {
  await query(
    `INSERT INTO runs (
      id, dataset, dataset_path, max_frames, output_path, status
    ) VALUES ($1, $2, $3, $4, $5, $6)`,
    [
      run.id,
      run.dataset,
      run.datasetPath,
      run.maxFrames,
      run.outputPath,
      run.status
    ]
  );
}

async function updateRunStatus(runId, status, fields = {}) {
  const sets = ["status = $2", "updated_at = NOW()"];
  const values = [runId, status];
  let index = 3;

  if (fields.errorMessage !== undefined) {
    sets.push(`error_message = $${index}`);
    values.push(fields.errorMessage);
    index += 1;
  }
  if (fields.exitCode !== undefined) {
    sets.push(`exit_code = $${index}`);
    values.push(fields.exitCode);
    index += 1;
  }
  if (fields.startedAtNow) {
    sets.push("started_at = NOW()");
  }
  if (fields.completedAtNow) {
    sets.push("completed_at = NOW()");
  }

  await query(`UPDATE runs SET ${sets.join(", ")} WHERE id = $1`, values);
}

async function addRunEvent(runId, level, message) {
  await query(
    "INSERT INTO run_events (run_id, level, message) VALUES ($1, $2, $3)",
    [runId, level, message]
  );
}

async function replaceRunTrajectory(runId, poses) {
  await query("DELETE FROM run_poses WHERE run_id = $1", [runId]);
  for (const pose of poses) {
    await query(
      `INSERT INTO run_poses (
          run_id, pose_index, timestamp_s, px, py, pz, qx, qy, qz, qw
       ) VALUES ($1, $2, $3, $4, $5, $6, $7, $8, $9, $10)`,
      [
        runId,
        pose.index,
        pose.timestampS,
        pose.position.x,
        pose.position.y,
        pose.position.z,
        pose.orientation.x,
        pose.orientation.y,
        pose.orientation.z,
        pose.orientation.w
      ]
    );
  }
}

async function replaceRunMetrics(runId, metrics) {
  await query("DELETE FROM run_metrics WHERE run_id = $1", [runId]);
  for (const metric of metrics) {
    await query(
      `INSERT INTO run_metrics (
          run_id, frame_index, timestamp_s, translation_step, estimated_speed, confidence, drift_risk
       ) VALUES ($1, $2, $3, $4, $5, $6, $7)`,
      [
        runId,
        metric.frameIndex,
        metric.timestampS,
        metric.translationStep,
        metric.estimatedSpeed,
        metric.confidence,
        metric.driftRisk
      ]
    );
  }
}

async function listRuns(limit = 20) {
  const result = await query(
    `SELECT id, dataset, dataset_path AS "datasetPath", max_frames AS "maxFrames",
            output_path AS "outputPath", status, error_message AS "errorMessage",
            exit_code AS "exitCode", created_at AS "createdAt", started_at AS "startedAt",
            completed_at AS "completedAt", updated_at AS "updatedAt"
       FROM runs
      ORDER BY created_at DESC
      LIMIT $1`,
    [limit]
  );
  return result.rows;
}

async function getRun(runId) {
  const result = await query(
    `SELECT id, dataset, dataset_path AS "datasetPath", max_frames AS "maxFrames",
            output_path AS "outputPath", status, error_message AS "errorMessage",
            exit_code AS "exitCode", created_at AS "createdAt", started_at AS "startedAt",
            completed_at AS "completedAt", updated_at AS "updatedAt"
       FROM runs
      WHERE id = $1`,
    [runId]
  );
  return result.rows[0] || null;
}

async function getRunTrajectory(runId) {
  const result = await query(
    `SELECT pose_index AS "index", timestamp_s AS "timestampS", px, py, pz, qx, qy, qz, qw
       FROM run_poses
      WHERE run_id = $1
      ORDER BY pose_index ASC`,
    [runId]
  );
  return result.rows;
}

async function getRunMetrics(runId) {
  const result = await query(
    `SELECT frame_index AS "frameIndex", timestamp_s AS "timestampS",
            translation_step AS "translationStep", estimated_speed AS "estimatedSpeed",
            confidence, drift_risk AS "driftRisk"
       FROM run_metrics
      WHERE run_id = $1
      ORDER BY frame_index ASC`,
    [runId]
  );
  return result.rows;
}

async function getRunEvents(runId, limit = 200) {
  const result = await query(
    `SELECT level, message, created_at AS "createdAt"
       FROM run_events
      WHERE run_id = $1
      ORDER BY id DESC
      LIMIT $2`,
    [runId, limit]
  );
  return result.rows.reverse();
}

module.exports = {
  addRunEvent,
  createRun,
  getRun,
  getRunEvents,
  getRunMetrics,
  getRunTrajectory,
  listRuns,
  replaceRunMetrics,
  replaceRunTrajectory,
  updateRunStatus
};

