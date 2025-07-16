package model

import "go.mongodb.org/mongo-driver/bson/primitive"

type GateType string

const (
	Input  GateType = "INPUT"
	Clock  GateType = "CLOCK"
	Output GateType = "OUTPUT"
	And    GateType = "AND"
	Or     GateType = "OR"
	Xor    GateType = "XOR"
	Xnor   GateType = "XNOR"
	Nand   GateType = "NAND"
	Nor    GateType = "NOR"
	Not    GateType = "NOT"
	Dff    GateType = "DFF"
	Dffn   GateType = "DFFN"
	Dlatch GateType = "DLATCH"
	Tff    GateType = "TFF"
	Jkff   GateType = "JKFF"
)

type GateID string

type Gate struct {
	ID        GateID   `bson:"id"`
	Type      GateType `bson:"type"`
	NumInputs int      `bson:"numInputs"`
	Inputs    []GateID `bson:"inputs"`
	Output    bool     `bson:"output"`
	NextState bool     `bson:"nextState"`
	PrevClk   bool     `bson:"prevClk"`
}

type Circuit struct {
	ID          primitive.ObjectID `bson:"_id,omitempty"`
	Name        string             `bson:"name"`
	Description string             `bson:"description"`
	UserID      primitive.ObjectID `bson:"userId"`
	Gates       []Gate             `bson:"gates"`
}
