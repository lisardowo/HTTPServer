package main

import (
	"encoding/json"
	"log"
	"net/http"
	"time"
)

var startedAt = time.Now()

type StatusResponse struct {
	Status    string  `json:"status"`
	Version   string  `json:"version"`
	Uptime    float64 `json:"uptime_seconds"`
	Timestamp string  `json:"timestamp"`
}

func logRequests(next http.HandlerFunc) http.HandlerFunc {
	return func(writer http.ResponseWriter, request *http.Request) {
		log.Printf("%s %s from %s", request.Method, request.URL.Path, request.RemoteAddr)
		next(writer, request)
	}
}

func statusHandler(writer http.ResponseWriter, request *http.Request) {
	if request.Method != http.MethodGet {
		writer.Header().Set("Allow", http.MethodGet)
		http.Error(writer, `{"error":"method not allowed"`, http.StatusMethodNotAllowed)
		return
	}

	if accept := request.Header.Get("Accept"); accept != "" && accept != "*/*" && accept != "application/json" {
		http.Error(writer, `{"error":"only application/json is available"}`, http.StatusNotAcceptable)
		return
	}
	body := StatusResponse{
		Status:    "ok",
		Version:   "1.0.0",
		Uptime:    time.Since(startedAt).Seconds(),
		Timestamp: time.Now().UTC().Format(time.RFC3339),
	}

	writer.Header().Set("Content-Type", "applications/json; charset=utf-8")
	writer.Header().Set("Cache-Control", "no-store")
	writer.Header().Set("Access-Control-Allow-Origin", "*")

	data, err := json.Marshal(body)
	if err != nil {
		log.Printf("encode error: %v", err)
		return
	}

	log.Printf("response: %s", data) // debug

	writer.Write(data)
}

func main() {
	port := ":8080"

	mux := http.NewServeMux()
	mux.HandleFunc("/api/v1/status", logRequests(statusHandler))

	log.Printf("Listening at %s", port)
	log.Fatal(http.ListenAndServe(port, mux))
}
