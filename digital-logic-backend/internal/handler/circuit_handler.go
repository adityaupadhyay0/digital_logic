package handler

import (
	"digital-logic-backend/internal/model"
	"digital-logic-backend/internal/service"
	"net/http"

	"github.com/gin-gonic/gin"
	"go.mongodb.org/mongo-driver/bson/primitive"
)

type CircuitHandler struct {
	circuitService *service.CircuitService
}

func NewCircuitHandler(circuitService *service.CircuitService) *CircuitHandler {
	return &CircuitHandler{
		circuitService: circuitService,
	}
}

type createCircuitRequest struct {
	Name        string       `json:"name" binding:"required"`
	Description string       `json:"description"`
	Gates       []model.Gate `json:"gates" binding:"required"`
}

func (h *CircuitHandler) CreateCircuit(c *gin.Context) {
	var req createCircuitRequest
	if err := c.ShouldBindJSON(&req); err != nil {
		c.JSON(http.StatusBadRequest, gin.H{"error": err.Error()})
		return
	}

	userID, _ := c.Get("userID")

	if err := h.circuitService.CreateCircuit(req.Name, req.Description, userID.(primitive.ObjectID), req.Gates); err != nil {
		c.JSON(http.StatusInternalServerError, gin.H{"error": err.Error()})
		return
	}

	c.JSON(http.StatusCreated, gin.H{"message": "Circuit created successfully"})
}

func (h *CircuitHandler) GetCircuits(c *gin.Context) {
	userID, _ := c.Get("userID")

	circuits, err := h.circuitService.GetCircuitsByUserID(userID.(primitive.ObjectID))
	if err != nil {
		c.JSON(http.StatusInternalServerError, gin.H{"error": err.Error()})
		return
	}

	c.JSON(http.StatusOK, circuits)
}

func (h *CircuitHandler) GetCircuit(c *gin.Context) {
	id := c.Param("id")
	objID, err := primitive.ObjectIDFromHex(id)
	if err != nil {
		c.JSON(http.StatusBadRequest, gin.H{"error": "Invalid circuit ID"})
		return
	}

	circuit, err := h.circuitService.GetCircuitByID(objID)
	if err != nil {
		c.JSON(http.StatusNotFound, gin.H{"error": "Circuit not found"})
		return
	}

	c.JSON(http.StatusOK, circuit)
}

type updateCircuitRequest struct {
	Name        string       `json:"name"`
	Description string       `json:"description"`
	Gates       []model.Gate `json:"gates"`
}

func (h *CircuitHandler) UpdateCircuit(c *gin.Context) {
	id := c.Param("id")
	objID, err := primitive.ObjectIDFromHex(id)
	if err != nil {
		c.JSON(http.StatusBadRequest, gin.H{"error": "Invalid circuit ID"})
		return
	}

	var req updateCircuitRequest
	if err := c.ShouldBindJSON(&req); err != nil {
		c.JSON(http.StatusBadRequest, gin.H{"error": err.Error()})
		return
	}

	circuit, err := h.circuitService.GetCircuitByID(objID)
	if err != nil {
		c.JSON(http.StatusNotFound, gin.H{"error": "Circuit not found"})
		return
	}

	if req.Name != "" {
		circuit.Name = req.Name
	}
	if req.Description != "" {
		circuit.Description = req.Description
	}
	if req.Gates != nil {
		circuit.Gates = req.Gates
	}

	if err := h.circuitService.UpdateCircuit(circuit); err != nil {
		c.JSON(http.StatusInternalServerError, gin.H{"error": err.Error()})
		return
	}

	c.JSON(http.StatusOK, gin.H{"message": "Circuit updated successfully"})
}

func (h *CircuitHandler) DeleteCircuit(c *gin.Context) {
	id := c.Param("id")
	objID, err := primitive.ObjectIDFromHex(id)
	if err != nil {
		c.JSON(http.StatusBadRequest, gin.H{"error": "Invalid circuit ID"})
		return
	}

	if err := h.circuitService.DeleteCircuit(objID); err != nil {
		c.JSON(http.StatusInternalServerError, gin.H{"error": err.Error()})
		return
	}

	c.JSON(http.StatusOK, gin.H{"message": "Circuit deleted successfully"})
}

type simulateCircuitRequest struct {
	Inputs map[model.GateID]bool `json:"inputs"`
	Cycles int                   `json:"cycles"`
}

func (h *CircuitHandler) SimulateCircuit(c *gin.Context) {
	id := c.Param("id")
	objID, err := primitive.ObjectIDFromHex(id)
	if err != nil {
		c.JSON(http.StatusBadRequest, gin.H{"error": "Invalid circuit ID"})
		return
	}

	var req simulateCircuitRequest
	if err := c.ShouldBindJSON(&req); err != nil {
		c.JSON(http.StatusBadRequest, gin.H{"error": err.Error()})
		return
	}

	result, err := h.circuitService.Simulate(objID, req.Inputs, req.Cycles)
	if err != nil {
		c.JSON(http.StatusInternalServerError, gin.H{"error": err.Error()})
		return
	}

	c.JSON(http.StatusOK, result)
}
