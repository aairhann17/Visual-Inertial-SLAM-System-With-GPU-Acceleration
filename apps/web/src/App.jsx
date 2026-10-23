import { useEffect, useMemo, useState } from "react";
import {
  cancelRun,
  createRun,
  fetchRun,
  fetchRunLogs,
  fetchRunMetrics,
  fetchRuns,
  fetchRunTrajectory,
  getRunStream
} from "./api";

function statusBadge(status) {
  const normalized = status || "unknown";
  return <span className={`badge ${normalized}`}>{normalized}</span>;
}

function TrajectoryPlot({ poses }) {
  const points = useMemo(() => {
    if (!poses.length) return [];
    const xs = poses.map((p) => p.px);
    const ys = poses.map((p) => p.py);
    const minX = Math.min(...xs);
    const maxX = Math.max(...xs);
    const minY = Math.min(...ys);
    const maxY = Math.max(...ys);
    const width = Math.max(1e-6, maxX - minX);
    const height = Math.max(1e-6, maxY - minY);

    return poses.map((p) => ({
      x: ((p.px - minX) / width) * 580 + 10,
      y: 260 - ((p.py - minY) / height) * 240
    }));
  }, [poses]);

  if (!points.length) {
    return <p className="muted">No trajectory data available yet.</p>;
  }

  const polyline = points.map((p) => `${p.x},${p.y}`).join(" ");
  return (
    <svg viewBox="0 0 600 280" className="trajectory-plot" role="img" aria-label="trajectory plot">
      <rect x="0" y="0" width="600" height="280" fill="#0b1220" />
      <polyline points={polyline} fill="none" stroke="#3b82f6" strokeWidth="2" />
      <circle cx={points[0].x} cy={points[0].y} r="4" fill="#22c55e" />
      <circle cx={points[points.length - 1].x} cy={points[points.length - 1].y} r="4" fill="#ef4444" />
    </svg>
  );
}

export function App() {
  const [runs, setRuns] = useState([]);
  const [selectedRunId, setSelectedRunId] = useState("");
  const [selectedRun, setSelectedRun] = useState(null);
  const [logs, setLogs] = useState([]);
  const [poses, setPoses] = useState([]);
  const [metrics, setMetrics] = useState([]);
  const [loading, setLoading] = useState(false);
  const [error, setError] = useState("");

  const [dataset, setDataset] = useState("euroc");
  const [datasetPath, setDatasetPath] = useState(
    "datasets/euroc/machine_hall/machine_hall/MH_01_easy"
  );
  const [maxFrames, setMaxFrames] = useState(300);

  async function refreshRuns() {
    const response = await fetchRuns();
    setRuns(response.runs);
    if (!selectedRunId && response.runs.length) {
      setSelectedRunId(response.runs[0].id);
    }
  }

  async function loadRunDetails(runId) {
    if (!runId) return;
    setLoading(true);
    try {
      const [run, runLogs, trajectory, runMetrics] = await Promise.all([
        fetchRun(runId),
        fetchRunLogs(runId),
        fetchRunTrajectory(runId),
        fetchRunMetrics(runId)
      ]);
      setSelectedRun(run);
      setLogs(runLogs.events);
      setPoses(trajectory.poses);
      setMetrics(runMetrics.metrics);
      setError("");
    } catch (err) {
      setError(err.message);
    } finally {
      setLoading(false);
    }
  }

  useEffect(() => {
    refreshRuns().catch((err) => setError(err.message));
  }, []);

  useEffect(() => {
    loadRunDetails(selectedRunId).catch((err) => setError(err.message));
  }, [selectedRunId]);

  useEffect(() => {
    if (!selectedRunId) return;
    const stream = getRunStream(selectedRunId);
    stream.addEventListener("status", () => {
      loadRunDetails(selectedRunId).catch((err) => setError(err.message));
      refreshRuns().catch((err) => setError(err.message));
    });
    stream.addEventListener("log", () => {
      loadRunDetails(selectedRunId).catch((err) => setError(err.message));
    });
    stream.onerror = () => {
      stream.close();
    };
    return () => stream.close();
  }, [selectedRunId]);

  async function handleRunSubmit(event) {
    event.preventDefault();
    setLoading(true);
    try {
      const created = await createRun({
        dataset,
        datasetPath,
        maxFrames: Number(maxFrames)
      });
      await refreshRuns();
      setSelectedRunId(created.runId);
      setError("");
    } catch (err) {
      setError(err.message);
    } finally {
      setLoading(false);
    }
  }

  async function handleCancel() {
    if (!selectedRunId) return;
    try {
      await cancelRun(selectedRunId);
      await refreshRuns();
      await loadRunDetails(selectedRunId);
    } catch (err) {
      setError(err.message);
    }
  }

  return (
    <div className="layout">
      <header>
        <h1>Visual-Inertial SLAM Dashboard</h1>
        <p className="muted">Run orchestration, trajectory review, and ML confidence monitoring.</p>
      </header>

      <section className="card">
        <h2>Start New Run</h2>
        <form onSubmit={handleRunSubmit} className="grid-form">
          <label>
            Dataset
            <input value={dataset} onChange={(e) => setDataset(e.target.value)} />
          </label>
          <label>
            Dataset Path
            <input value={datasetPath} onChange={(e) => setDatasetPath(e.target.value)} />
          </label>
          <label>
            Max Frames
            <input
              type="number"
              min="1"
              value={maxFrames}
              onChange={(e) => setMaxFrames(e.target.value)}
            />
          </label>
          <button type="submit" disabled={loading}>
            {loading ? "Submitting..." : "Start Run"}
          </button>
        </form>
      </section>

      <section className="card">
        <div className="row">
          <h2>Runs</h2>
          <button onClick={() => refreshRuns().catch((err) => setError(err.message))}>Refresh</button>
        </div>
        <div className="runs-table">
          <table>
            <thead>
              <tr>
                <th>Run ID</th>
                <th>Dataset</th>
                <th>Status</th>
                <th>Created</th>
              </tr>
            </thead>
            <tbody>
              {runs.map((run) => (
                <tr
                  key={run.id}
                  className={run.id === selectedRunId ? "selected" : ""}
                  onClick={() => setSelectedRunId(run.id)}
                >
                  <td>{run.id.slice(0, 8)}</td>
                  <td>{run.dataset}</td>
                  <td>{statusBadge(run.status)}</td>
                  <td>{new Date(run.createdAt).toLocaleString()}</td>
                </tr>
              ))}
            </tbody>
          </table>
        </div>
      </section>

      <section className="card">
        <div className="row">
          <h2>Run Details</h2>
          <button onClick={handleCancel} disabled={!selectedRunId}>
            Cancel Run
          </button>
        </div>
        {selectedRun ? (
          <div className="details-grid">
            <p><strong>ID:</strong> {selectedRun.id}</p>
            <p><strong>Status:</strong> {statusBadge(selectedRun.status)}</p>
            <p><strong>Dataset path:</strong> {selectedRun.datasetPath}</p>
            <p><strong>Output:</strong> {selectedRun.outputPath}</p>
            {selectedRun.errorMessage ? <p className="error">{selectedRun.errorMessage}</p> : null}
          </div>
        ) : (
          <p className="muted">Select a run to view details.</p>
        )}
      </section>

      <section className="card">
        <h2>Trajectory</h2>
        <TrajectoryPlot poses={poses} />
      </section>

      <section className="card">
        <h2>ML Confidence Metrics</h2>
        <div className="runs-table">
          <table>
            <thead>
              <tr>
                <th>Frame</th>
                <th>Time (s)</th>
                <th>Step</th>
                <th>Speed</th>
                <th>Confidence</th>
                <th>Risk</th>
              </tr>
            </thead>
            <tbody>
              {metrics.slice(0, 50).map((metric) => (
                <tr key={metric.frameIndex}>
                  <td>{metric.frameIndex}</td>
                  <td>{metric.timestampS.toFixed(3)}</td>
                  <td>{metric.translationStep.toFixed(6)}</td>
                  <td>{metric.estimatedSpeed.toFixed(6)}</td>
                  <td>{metric.confidence.toFixed(3)}</td>
                  <td>{metric.driftRisk}</td>
                </tr>
              ))}
            </tbody>
          </table>
        </div>
      </section>

      <section className="card">
        <h2>Logs</h2>
        <div className="logs">
          {logs.map((event, idx) => (
            <div key={`${event.createdAt}-${idx}`}>
              <span className="muted">{new Date(event.createdAt).toLocaleTimeString()}</span>{" "}
              <strong>{event.level}:</strong> {event.message}
            </div>
          ))}
        </div>
      </section>

      {error ? <p className="error">{error}</p> : null}
    </div>
  );
}

