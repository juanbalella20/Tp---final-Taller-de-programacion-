#ifndef GAME_EXCEPTIONS_H_
#define GAME_EXCEPTIONS_H_

#include <stdexcept>
#include <string>

class NoEntityException : public std::runtime_error {
public:
    NoEntityException() : std::runtime_error("No hay enemigo ahí") {}
};

class AttackerNotFoundException : public std::runtime_error {
public:
    AttackerNotFoundException() : std::runtime_error("Atacante no encontrado en el mapa") {}
};

class AttackNotAllowedException : public std::runtime_error {
public:
    explicit AttackNotAllowedException(const std::string& reason)
        : std::runtime_error(reason) {}
};

class NoWeaponEquippedException : public std::runtime_error {
public:
    NoWeaponEquippedException() : std::runtime_error("No tenés arma equipada") {}
};

class OutOfRangeException : public std::runtime_error {
public:
    OutOfRangeException() : std::runtime_error("El enemigo está fuera de rango") {}
};

#endif
