const API_BASE = import.meta.env.VITE_API_BASE_URL || "http://localhost:8080/api";

async function parseJson(response) {
  const body = await response.json().catch(() => ({}));
  if (!response.ok) {
    throw new Error(body.message || body.error || `Request failed: ${response.status}`);
  }
  return body;
}

export async function createRun(payload) {
  const response = await fetch(`${API_BASE}/runs`, {
    method: "POST",
    headers: { "Content-Type": "application/json" },
    body: JSON.stringify(payload)
  });
  return parseJson(response);
}

export async function fetchRuns(limit = 25) {
  const response = await fetch(`${API_BASE}/runs?limit=${limit}`);
  return parseJson(response);
}

export async function fetchRun(runId) {
  const response = await fetch(`${API_BASE}/runs/${runId}`);
  return parseJson(response);
}

export async function fetchRunTrajectory(runId) {
  const response = await fetch(`${API_BASE}/runs/${runId}/trajectory`);
  return parseJson(response);
}

export async function fetchRunMetrics(runId) {
  const response = await fetch(`${API_BASE}/runs/${runId}/metrics`);
  return parseJson(response);
}

export async function fetchRunLogs(runId) {
  const response = await fetch(`${API_BASE}/runs/${runId}/logs`);
  return parseJson(response);
}

export async function cancelRun(runId) {
  const response = await fetch(`${API_BASE}/runs/${runId}/cancel`, {
    method: "POST"
  });
  return parseJson(response);
}

export function getRunStream(runId) {
  return new EventSource(`${API_BASE}/runs/${runId}/stream`);
}

