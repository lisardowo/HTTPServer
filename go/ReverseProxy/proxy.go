package main

import (
	"log"
	"net/http"
	"net/http/httputil"
	"net/url"
	"strings"
)

type route struct {
	prefix  string
	backend *url.URL
	proxy   *httputil.ReverseProxy
}

type gateway struct {
	routes []route
}

func mustUrl(raw string) *url.URL {
	url, err := url.Parse(raw)
	if err != nil {
		log.Fatalf("URL error %q: %v", raw, err)
	}
	return url
}

func newRoute(prefix, backendURL string) route {
	url := mustUrl(backendURL)
	proxy := httputil.NewSingleHostReverseProxy(url)
	proxy.ErrorHandler = func(writer http.ResponseWriter, request *http.Request, err error) {
		log.Printf("%s unreachable: %v", url, err)
		http.Error(writer, "502 bad gateway: backend unavailable", http.StatusBadGateway)
	}
	return route{prefix: prefix, backend: url, proxy: proxy}
}

func (gateway *gateway) Match(path string) *route {
	seg := strings.SplitN(strings.TrimPrefix(path, "/"), "/", 2)[0]
	for i := range gateway.routes {
		if gateway.routes[i].prefix == seg {
			return &gateway.routes[i]
		}
	}
	return nil
}

func (gateway *gateway) ServeHTTP(writer http.ResponseWriter, request *http.Request) {
	route := gateway.Match(request.URL.Path)
	if route == nil {
		log.Printf("No route for %s %s", request.Method, request.URL.Path)
		http.NotFound(writer, request)
		return
	}
	log.Printf("%s %s -> %s", request.Method, request.URL.Path, route.backend)

	request.Header.Set("X-Forwarded-For", request.RemoteAddr)
	request.Header.Set("X-Gateway-Route", route.prefix)
	route.proxy.ServeHTTP(writer, request)
}

func main() {
	gateway := &gateway{routes: []route{
		newRoute("users", "http://localhost:8082"),
		newRoute("payments", "http://localhost:8083"),
	}}
	log.Printf("API gateway listening on :8081")
	log.Fatal(http.ListenAndServe(":8081", gateway))
}
