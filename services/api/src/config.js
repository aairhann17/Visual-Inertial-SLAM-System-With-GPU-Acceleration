const path = require("path");

function parseBoolean(value, fallback) {
  if (value === undefined) {
    return fallback;
  }
  return value.toLowerCase() === "true";
}

const rootDir = path.resolve(__dirname, "..", "..", "..");

const config = {
  port: Number(process.env.PORT || 8080),
  databaseUrl: process.env.DATABASE_URL || "",
  vislamExecutable:
    process.env.VISLAM_EXECUTABLE ||
    path.join(rootDir, "build", "Debug", "vislam_app.exe"),
  artifactsDir:
    process.env.ARTIFACTS_DIR || path.join(rootDir, "artifacts", "runs"),
  enableMlConfidence: parseBoolean(process.env.ENABLE_ML_CONFIDENCE, true)
};

module.exports = { config, rootDir };

