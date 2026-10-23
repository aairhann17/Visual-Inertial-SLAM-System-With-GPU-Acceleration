const fs = require("fs");

function clamp(value, min, max) {
  return Math.max(min, Math.min(max, value));
}

function loadModel(modelPath) {
  if (!modelPath) {
    return null;
  }
  if (!fs.existsSync(modelPath)) {
    return null;
  }

  const raw = fs.readFileSync(modelPath, "utf8");
  const parsed = JSON.parse(raw);
  if (
    typeof parsed.base !== "number" ||
    typeof parsed.weights?.translation_step !== "number" ||
    typeof parsed.weights?.estimated_speed !== "number"
  ) {
    throw new Error("Invalid confidence model file format.");
  }
  return parsed;
}

function sigmoid(x) {
  return 1 / (1 + Math.exp(-x));
}

function computeConfidenceFromHeuristics(metric) {
  let score = 0.88;
  score -= clamp(metric.translationStep * 2.3, 0, 0.65);
  score -= clamp(Math.max(0, metric.estimatedSpeed - 0.03) * 2.2, 0, 0.4);
  return clamp(score, 0.05, 0.99);
}

function computeConfidence(model, metric) {
  if (!model) {
    return computeConfidenceFromHeuristics(metric);
  }

  const linear =
    model.base +
    metric.translationStep * model.weights.translation_step +
    metric.estimatedSpeed * model.weights.estimated_speed;
  return clamp(sigmoid(linear), 0.01, 0.99);
}

function classifyRisk(confidence) {
  if (confidence < 0.5) return "likely_drift";
  if (confidence < 0.75) return "warning";
  return "stable";
}

module.exports = {
  classifyRisk,
  computeConfidence,
  loadModel
};

