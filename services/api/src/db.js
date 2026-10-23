const { Pool } = require("pg");
const { config } = require("./config");

if (!config.databaseUrl) {
  throw new Error(
    "DATABASE_URL is required for the API service. Set it in environment variables."
  );
}

const pool = new Pool({
  connectionString: config.databaseUrl
});

async function query(text, params = []) {
  return pool.query(text, params);
}

async function closePool() {
  await pool.end();
}

module.exports = { query, closePool };

