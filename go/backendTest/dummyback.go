package main

import (
	"flag"
	"fmt"
	"log"
	"net/http"
)

func main() {
	name := flag.String("name", "backend", "service name")
	port := flag.Int("port", 8082, "listen port")

	flag.Parse()

	handler := func(writer http.ResponseWriter, request *http.Request) {
		log.Printf("%s %s %s (Host=%s, X-Forwarded-For=%q", *name, request.Method, request.URL.Path, request.Host, request.Header.Get("X-Forwarded-For"))
		writer.Header().Set("Content-Type", "text/plain; charset=utf-8")
		writer.Header().Set("X-Serverd-By", *name)
		fmt.Fprint(writer, "serverd by %s on port %d, path %s\n", *name, *port, request.URL.Path)
	}

	addr := fmt.Sprint("127.0.0.1:%d", *port)
	log.Printf("Listening at %s", addr)
	log.Fatal(http.ListenAndServe(addr, http.HandlerFunc(handler)))
}
