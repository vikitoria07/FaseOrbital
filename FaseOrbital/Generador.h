// Generador.h
#pragma once
#include <cstdint>

class GeneradorPseudoaleatorio {
private:
    std::uint32_t estado_actual;

public:
    explicit GeneradorPseudoaleatorio(std::uint64_t semilla)
        : estado_actual(static_cast<std::uint32_t>(semilla & 0x7FFFFFFFULL)) {
    }

    std::uint32_t siguiente() {
        estado_actual = static_cast<std::uint32_t>(
            (1103515245ULL * estado_actual + 12345ULL) & 0x7FFFFFFFULL
            );
        return estado_actual;
    }
};