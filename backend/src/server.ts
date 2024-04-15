import * as cors from "cors";
import express from "express";
import http from "http";
import { Server } from "socket.io";
import sqlite3 from "sqlite3";

const app = express();
const server = http.createServer(app);
const io = new Server(server, {
  allowEIO3: true,
  cors: {
    origin: "*",
    methods: ["GET", "POST"],
  },
});
const db = new sqlite3.Database("database.db");

app.use(express.json());
app.use(cors.default());
// Events

app.get("/", (req, res) => {
  res.send("Hello World");
});

io.on("connection", (socket) => {
  console.log("a user connected");

  socket.on("event", (data) => {
    console.log("Received event: ", data);
    socket.broadcast.emit("event", data);
    // addEventInDatabase({
    //   type: data.type,
    //   value: data.value,
    //   timestamp: Date.now(),
    // });
  });

  socket.on("action", (data) => {
    console.log("Received action: ", data);
    socket.broadcast.emit("action", data);
  });
});

io.engine.on("connection_error", (err) => {
  console.log(err.req); // the request object
  console.log(err.code); // the error code, for example 1
  console.log(err.message); // the error message, for example "Session ID unknown"
  console.log(err.context); // some additional error context
});

io.on("connect_error", (err) => {
  console.log(`connect_error due to ${err.message}`);
});

interface Event {
  type: string;
  value: string;
  timestamp: number;
}
function addEventInDatabase(event: Event) {
  db.run("INSERT INTO events (type, value, timestamp) VALUES (?, ?, ?)", [
    event.type,
    event.value,
    event.timestamp,
  ]);
}

app.get("/events/:limit", (req, res) => {
  db.all<Event>(
    "SELECT * FROM events ORDER BY timestamp DESC LIMIT ?",
    [req.params.limit],
    (err, rows) => {
      if (err) {
        res.status(500).send("Internal server error");
        return;
      }

      const events = rows.map((row) => ({
        type: row.type,
        value: row.value,
        timestamp: row.timestamp,
      }));

      res.json(events);
    }
  );
});

server.listen(3000, () => {
  console.log("Socket server and app is running on http://localhost:3000");
  db.run(
    "CREATE TABLE IF NOT EXISTS events (id INTEGER PRIMARY KEY, type TEXT, value TEXT, timestamp INTEGER)"
  );
});
