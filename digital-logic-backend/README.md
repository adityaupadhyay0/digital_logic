# Digital Logic Backend

This is the backend for the digital logic simulator. It is built with Go, Gin, and MongoDB.

## Getting Started

1.  Create a `.env` file in the root directory with the following variables:
    ```
    MONGO_URI=mongodb://localhost:27017
    PORT=8080
    ```
2.  Install the dependencies:
    ```
    go mod tidy
    ```
3.  Run the server:
    ```
    go run cmd/server/main.go
    ```

## API Endpoints

### Authentication

*   `POST /auth/register`: Register a new user.
*   `POST /auth/login`: Log in a user.

### Circuits

*   `POST /api/circuits`: Create a new circuit.
*   `GET /api/circuits`: Get all circuits for the logged-in user.
*   `GET /api/circuits/:id`: Get a specific circuit.
*   `PUT /api/circuits/:id`: Update a circuit.
*   `DELETE /api/circuits/:id`: Delete a circuit.
*   `POST /api/circuits/:id/simulate`: Simulate a circuit.
