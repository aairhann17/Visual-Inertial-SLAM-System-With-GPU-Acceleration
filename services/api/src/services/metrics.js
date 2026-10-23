const {
  classifyRisk,
  computeConfidence
} = require("../ml/confidence");

function euclideanDistance(a, b) {
  const dx = b.position.x - a.position.x;
  const dy = b.position.y - a.position.y;
  const dz = b.position.z - a.position.z;
  return Math.sqrt(dx * dx + dy * dy + dz * dz);
}

function buildMetrics(poses, { model, mlEnabled }) {
  if (poses.length === 0) return [];

  const metrics = [];
  let previous = poses[0];

  metrics.push({
    frameIndex: 0,
    timestampS: previous.timestampS,
    translationStep: 0,
    estimatedSpeed: 0,
    confidence: mlEnabled ? computeConfidence(model, { translationStep: 0, estimatedSpeed: 0 }) : 1.0
  });

  for (let i = 1; i < poses.length; i += 1) {
    const current = poses[i];
    const dt = Math.max(1e-6, current.timestampS - previous.timestampS);
    const translationStep = euclideanDistance(previous, current);
    const estimatedSpeed = translationStep / dt;
    const confidence = mlEnabled
      ? computeConfidence(model, { translationStep, estimatedSpeed })
      : 1.0;

    metrics.push({
      frameIndex: i,
      timestampS: current.timestampS,
      translationStep,
      estimatedSpeed,
      confidence
    });

    previous = current;
  }

  return metrics.map((metric) => ({
    ...metric,
    driftRisk: classifyRisk(metric.confidence)
  }));
}

module.exports = { buildMetrics };

