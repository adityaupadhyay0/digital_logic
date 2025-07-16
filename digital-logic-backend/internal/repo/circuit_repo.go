package repo

import (
	"context"
	"digital-logic-backend/internal/model"

	"go.mongodb.org/mongo-driver/bson"
	"go.mongodb.org/mongo-driver/bson/primitive"
	"go.mongodb.org/mongo-driver/mongo"
)

type CircuitRepo struct {
	collection *mongo.Collection
}

func NewCircuitRepo(db *mongo.Database) *CircuitRepo {
	return &CircuitRepo{
		collection: db.Collection("circuits"),
	}
}

func (r *CircuitRepo) CreateCircuit(circuit *model.Circuit) error {
	_, err := r.collection.InsertOne(context.Background(), circuit)
	return err
}

func (r *CircuitRepo) GetCircuitsByUserID(userID primitive.ObjectID) ([]*model.Circuit, error) {
	var circuits []*model.Circuit
	cursor, err := r.collection.Find(context.Background(), bson.M{"userId": userID})
	if err != nil {
		return nil, err
	}
	defer cursor.Close(context.Background())

	for cursor.Next(context.Background()) {
		var circuit model.Circuit
		if err := cursor.Decode(&circuit); err != nil {
			return nil, err
		}
		circuits = append(circuits, &circuit)
	}

	return circuits, nil
}

func (r *CircuitRepo) GetCircuitByID(id primitive.ObjectID) (*model.Circuit, error) {
	var circuit model.Circuit
	err := r.collection.FindOne(context.Background(), bson.M{"_id": id}).Decode(&circuit)
	if err != nil {
		return nil, err
	}
	return &circuit, nil
}

func (r *CircuitRepo) UpdateCircuit(circuit *model.Circuit) error {
	_, err := r.collection.UpdateOne(
		context.Background(),
		bson.M{"_id": circuit.ID},
		bson.M{"$set": circuit},
	)
	return err
}

func (r *CircuitRepo) DeleteCircuit(id primitive.ObjectID) error {
	_, err := r.collection.DeleteOne(context.Background(), bson.M{"_id": id})
	return err
}
