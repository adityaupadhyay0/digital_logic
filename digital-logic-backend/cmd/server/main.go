package main

import (
	"digital-logic-backend/config"
	"digital-logic-backend/internal/handler"
	"digital-logic-backend/internal/repo"
	"digital-logic-backend/internal/routes"
	"digital-logic-backend/internal/service"
	"log"

	"github.com/gin-gonic/gin"
)

func main() {
	cfg := config.LoadConfig()
	db := config.ConnectDB(cfg)

	userRepo := repo.NewUserRepo(db)
	circuitRepo := repo.NewCircuitRepo(db)

	authService := service.NewAuthService(userRepo)
	circuitService := service.NewCircuitService(circuitRepo)

	authHandler := handler.NewAuthHandler(authService)
	circuitHandler := handler.NewCircuitHandler(circuitService)

	router := gin.Default()
	routes.SetupRoutes(router, userRepo, authHandler, circuitHandler)

	log.Printf("Server starting on port %s", cfg.Port)
	if err := router.Run(":" + cfg.Port); err != nil {
		log.Fatal(err)
	}
}
