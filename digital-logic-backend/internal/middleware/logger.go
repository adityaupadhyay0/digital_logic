package middleware

import (
	"log"
	"time"

	"github.com/gin-gonic/gin"
)

func Logger() gin.HandlerFunc {
	return func(c *gin.Context) {
		start := time.Now()
		c.Next()
		duration := time.Since(start)
		log.Printf("method=%s path=%s status=%d duration=%s", c.Request.Method, c.Request.URL.Path, c.Writer.Status(), duration)
	}
}
