package service

import (
	"digital-logic-backend/internal/model"
	"digital-logic-backend/internal/repo"
	"digital-logic-backend/pkg/hashutil"
	"digital-logic-backend/pkg/token"
)

type AuthService struct {
	userRepo *repo.UserRepo
}

func NewAuthService(userRepo *repo.UserRepo) *AuthService {
	return &AuthService{
		userRepo: userRepo,
	}
}

func (s *AuthService) Register(username, password string) error {
	hashedPassword, err := hashutil.HashPassword(password)
	if err != nil {
		return err
	}

	user := &model.User{
		Username: username,
		Password: hashedPassword,
	}

	return s.userRepo.CreateUser(user)
}

func (s *AuthService) Login(username, password string) (string, error) {
	user, err := s.userRepo.GetUserByUsername(username)
	if err != nil {
		return "", err
	}

	if !hashutil.CheckPasswordHash(password, user.Password) {
		return "", err
	}

	return token.GenerateToken(username)
}
