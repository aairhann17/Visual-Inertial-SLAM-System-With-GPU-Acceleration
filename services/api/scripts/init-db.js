require("dotenv").config();

const fs = require("fs");
const path = require("path");
const { query, closePool } = require("../src/db");

async function main() {
  const schemaPath = path.resolve(__dirname, "..", "src", "sql", "schema.sql");
  const schemaSql = fs.readFileSync(schemaPath, "utf8");
  await query(schemaSql);
  console.log("Database schema initialized.");
}

main()
  .catch((error) => {
    console.error("Failed to initialize database schema:", error);
    process.exitCode = 1;
  })
  .finally(async () => {
    await closePool();
  });

