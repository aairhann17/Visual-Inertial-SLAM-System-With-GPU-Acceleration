CREATE TABLE IF NOT EXISTS runs (
    id UUID PRIMARY KEY,
    dataset TEXT NOT NULL,
    dataset_path TEXT NOT NULL,
    max_frames INTEGER NOT NULL,
    output_path TEXT NOT NULL,
    status TEXT NOT NULL,
    error_message TEXT,
    exit_code INTEGER,
    created_at TIMESTAMPTZ NOT NULL DEFAULT NOW(),
    started_at TIMESTAMPTZ,
    completed_at TIMESTAMPTZ,
    updated_at TIMESTAMPTZ NOT NULL DEFAULT NOW()
);

CREATE TABLE IF NOT EXISTS run_events (
    id BIGSERIAL PRIMARY KEY,
    run_id UUID NOT NULL REFERENCES runs(id) ON DELETE CASCADE,
    level TEXT NOT NULL,
    message TEXT NOT NULL,
    created_at TIMESTAMPTZ NOT NULL DEFAULT NOW()
);

CREATE INDEX IF NOT EXISTS idx_run_events_run_id_created_at ON run_events(run_id, created_at);

CREATE TABLE IF NOT EXISTS run_poses (
    id BIGSERIAL PRIMARY KEY,
    run_id UUID NOT NULL REFERENCES runs(id) ON DELETE CASCADE,
    pose_index INTEGER NOT NULL,
    timestamp_s DOUBLE PRECISION NOT NULL,
    px DOUBLE PRECISION NOT NULL,
    py DOUBLE PRECISION NOT NULL,
    pz DOUBLE PRECISION NOT NULL,
    qx DOUBLE PRECISION NOT NULL,
    qy DOUBLE PRECISION NOT NULL,
    qz DOUBLE PRECISION NOT NULL,
    qw DOUBLE PRECISION NOT NULL
);

CREATE INDEX IF NOT EXISTS idx_run_poses_run_id_index ON run_poses(run_id, pose_index);

CREATE TABLE IF NOT EXISTS run_metrics (
    id BIGSERIAL PRIMARY KEY,
    run_id UUID NOT NULL REFERENCES runs(id) ON DELETE CASCADE,
    frame_index INTEGER NOT NULL,
    timestamp_s DOUBLE PRECISION NOT NULL,
    translation_step DOUBLE PRECISION NOT NULL,
    estimated_speed DOUBLE PRECISION NOT NULL,
    confidence DOUBLE PRECISION NOT NULL,
    drift_risk TEXT NOT NULL
);

CREATE INDEX IF NOT EXISTS idx_run_metrics_run_id_index ON run_metrics(run_id, frame_index);

