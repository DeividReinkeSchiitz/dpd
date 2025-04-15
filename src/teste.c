#include <stdio.h>
#include <stdlib.h>
#include <time.h>

typedef struct {
    int codigo_turma, peso_atribuido;
} Candidatos;

// Função que retorna um Candidato aleatório do vetor
Candidatos escolherAleatorio(Candidatos *vetor, int tamanho) {
    if (tamanho == 0) {
        Candidatos vazio = {-1, -1}; // ou algum valor sentinela
        return vazio;
    }

    int indice = rand() % tamanho;
    return vetor[indice];
}

// Exemplo de uso
int main() {
    srand(time(NULL)); // Inicializa o gerador de números aleatórios

    Candidatos vetor[3] = {
        {101, 5},
        {102, 8},
        {103, 3}
    };

    Candidatos escolhido = escolherAleatorio(vetor, 3);
    printf("Turma escolhida: %d, Peso: %d\n", escolhido.codigo_turma, escolhido.peso_atribuido);

    return 0;
}
