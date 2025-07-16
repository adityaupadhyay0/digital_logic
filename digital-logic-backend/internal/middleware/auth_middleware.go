package middleware

import (
	"digital-logic-backend/internal/repo"
	"digital-logic-backend/pkg/token"
	"net/http"
	"strings"

	"github.com/gin-gonic/gin"
)

func AuthMiddleware(userRepo *repo.UserRepo) gin.HandlerFunc {
	return func(c *gin.Context) {
		authHeader := c.GetHeader("Authorization")
		if authHeader == "" {
			c.AbortWithStatusJSON(http.StatusUnauthorized, gin.H{"error": "Authorization header not provided"})
			return
		}

		tokenString := strings.TrimPrefix(authHeader, "Bearer ")
		if tokenString == authHeader {
			c.AbortWithStatusJSON(http.StatusUnauthorized, gin.H{"error": "Could not find bearer token in Authorization header"})
			return
		}

		claims, err := token.ValidateToken(tokenString)
		if err != nil {
			c.AbortWithStatusJSON(http.StatusUnauthorized, gin.H{"error": "Invalid token"})
			return
		}

		user, err := userRepo.GetUserByUsername(claims.Username)
		if err != nil {
			c.AbortWithStatusJSON(http.StatusUnauthorized, gin.H{"error": "User not found"})
			return
		}

		c.Set("userID", user.ID)
		c.Next()
	}
}
