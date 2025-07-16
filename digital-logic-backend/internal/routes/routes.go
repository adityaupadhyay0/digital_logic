package routes

import (
	"digital-logic-backend/internal/handler"
	"digital-logic-backend/internal/middleware"
	"digital-logic-backend/internal/repo"

	"github.com/gin-gonic/gin"
)

func SetupRoutes(router *gin.Engine, userRepo *repo.UserRepo, authHandler *handler.AuthHandler, circuitHandler *handler.CircuitHandler) {
	router.Use(middleware.Logger())

	authRoutes := router.Group("/auth")
	{
		authRoutes.POST("/register", authHandler.Register)
		authRoutes.POST("/login", authHandler.Login)
	}

	apiRoutes := router.Group("/api")
	apiRoutes.Use(middleware.AuthMiddleware(userRepo))
	{
		apiRoutes.POST("/circuits", circuitHandler.CreateCircuit)
		apiRoutes.GET("/circuits", circuitHandler.GetCircuits)
		apiRoutes.GET("/circuits/:id", circuitHandler.GetCircuit)
		apiRoutes.PUT("/circuits/:id", circuitHandler.UpdateCircuit)
		apiRoutes.DELETE("/circuits/:id", circuitHandler.DeleteCircuit)
		apiRoutes.POST("/circuits/:id/simulate", circuitHandler.SimulateCircuit)
	}
}
