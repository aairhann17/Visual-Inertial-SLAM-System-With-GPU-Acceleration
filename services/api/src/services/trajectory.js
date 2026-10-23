const fs = require("fs");

function parseTrajectoryLine(line, index) {
  const fields = line.trim().split(/\s+/);
  if (fields.length !== 8) {
    throw new Error(
      `Invalid trajectory row at index ${index}: expected 8 columns, got ${fields.length}.`
    );
  }

  const values = fields.map(Number);
  if (values.some((v) => Number.isNaN(v))) {
    throw new Error(`Non-numeric value in trajectory row at index ${index}.`);
  }

  return {
    index,
    timestampS: values[0],
    position: { x: values[1], y: values[2], z: values[3] },
    orientation: { x: values[4], y: values[5], z: values[6], w: values[7] }
  };
}

function readTrajectoryFile(trajectoryPath) {
  if (!fs.existsSync(trajectoryPath)) {
    throw new Error(`Trajectory file not found: ${trajectoryPath}`);
  }

  const raw = fs.readFileSync(trajectoryPath, "utf8");
  const lines = raw
    .split(/\r?\n/)
    .map((line) => line.trim())
    .filter((line) => line.length > 0);
  return lines.map(parseTrajectoryLine);
}

module.exports = { readTrajectoryFile };

