#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <assert.h>
#include <time.h>
#include "lib.h"
#define TAMANHO_DE_MEMORIA 0x0100
#define TAMANHO_DO_PREDITORDESVIO 0x0100

int estaRodando = 1;
int vaiPular = 1;
int vetorDeReg[8] = {0};
int pcBusca = 0;
int pcDecodificacao = -1;
int pcExecucao = -1;
int pc = 0;

typedef struct PreditorDeDesvio {
    int pc;
    int probabilidadeJump;
    int vezesJump;
    int vezesExecutado;
} PreditorDeDesvio;

PreditorDeDesvio vetorDoPreditorDesvio[TAMANHO_DO_PREDITORDESVIO];

typedef struct Instrucao {
    uint16_t formato;
    uint16_t opcodeR;
    uint16_t destino;
    uint16_t ope1;
    uint16_t ope2;
    uint16_t opcodeI;
    uint16_t registrador;
    uint16_t imediato;
} Instrucao;

uint16_t buscaInstrucao = 0;
Instrucao decodificaoInstrucao;
Instrucao execucaoInstrucao;

int randomInt(int min, int max) {
    return rand() % (max - min + 1) + min;
}

void atualizaPreditor(int pc, int pulou) {
    int index = pc % TAMANHO_DO_PREDITORDESVIO;
    
    if (vetorDoPreditorDesvio[index].pc != pc) {
        vetorDoPreditorDesvio[index].pc = pc;
        vetorDoPreditorDesvio[index].vezesJump = 0;
        vetorDoPreditorDesvio[index].vezesExecutado = 0;
        vetorDoPreditorDesvio[index].probabilidadeJump = 0;
    }

    vetorDoPreditorDesvio[index].vezesExecutado++;
    if (pulou) {
        vetorDoPreditorDesvio[index].vezesJump++;
    }

    int taxaDeJump = (vetorDoPreditorDesvio[index].vezesJump * 100) / vetorDoPreditorDesvio[index].vezesExecutado;
    if (taxaDeJump == 0) {
        vetorDoPreditorDesvio[index].probabilidadeJump = 0;
    } else if (taxaDeJump <= 25) {
        vetorDoPreditorDesvio[index].probabilidadeJump = 1;
    } else if (taxaDeJump <= 50) {
        vetorDoPreditorDesvio[index].probabilidadeJump = 2;
    } else {
        vetorDoPreditorDesvio[index].probabilidadeJump = 3;
    }
}

void busca(uint16_t *memoria, int pc) {
    buscaInstrucao = memoria[pc];
    int index = pc % TAMANHO_DO_PREDITORDESVIO;

    if (vetorDoPreditorDesvio[index].pc == pc) {
        int num = randomInt(0, 100);
        switch (vetorDoPreditorDesvio[index].probabilidadeJump) {
            case 0:
                break;
            case 1:
                if (num < 25) {
                    pc = pcExecucao + 1;
                }
                break;
            case 2:
                if (num < 50) {
                    pc = pcExecucao + 1;
                }
                break;
            case 3:
                pc = pcExecucao + 1;
                break;
        }
    }
}

Instrucao decodificacao(uint16_t instrucao) {
    Instrucao instrucoes;
    instrucoes.formato = extract_bits(instrucao, 15, 1);
    if (instrucoes.formato == 0) {
        instrucoes.opcodeR = extract_bits(instrucao, 9, 6);
        instrucoes.destino = extract_bits(instrucao, 6, 3);
        instrucoes.ope1 = extract_bits(instrucao, 3, 3);
        instrucoes.ope2 = extract_bits(instrucao, 0, 3);
    } else if (instrucoes.formato == 1) {
        instrucoes.opcodeI = extract_bits(instrucao, 13, 2);
        instrucoes.registrador = extract_bits(instrucao, 10, 3);
        instrucoes.imediato = extract_bits(instrucao, 0, 10);
    }
    return instrucoes;
}

void execucao(uint16_t *memoria, Instrucao conjuntoInstrucao) {
    vaiPular = 1;
    int pulou = 0;

    switch (conjuntoInstrucao.formato){
        case 0:
            switch (conjuntoInstrucao.opcodeR){
                case 0:
                    vetorDeReg[conjuntoInstrucao.destino] = vetorDeReg[conjuntoInstrucao.ope1] + vetorDeReg[conjuntoInstrucao.ope2];
                    break;
                case 1:
                    vetorDeReg[conjuntoInstrucao.destino] = vetorDeReg[conjuntoInstrucao.ope1] - vetorDeReg[conjuntoInstrucao.ope2];
                    break;
                case 2:
                    vetorDeReg[conjuntoInstrucao.destino] = vetorDeReg[conjuntoInstrucao.ope1] * vetorDeReg[conjuntoInstrucao.ope2];
                    break;
                case 3:
                    vetorDeReg[conjuntoInstrucao.destino] = vetorDeReg[conjuntoInstrucao.ope1] / vetorDeReg[conjuntoInstrucao.ope2];
                    break;
                case 4:
                    if(vetorDeReg[conjuntoInstrucao.ope1] == vetorDeReg[conjuntoInstrucao.ope2]){
                        vetorDeReg[conjuntoInstrucao.destino] = 1;
                    } else {
                        vetorDeReg[conjuntoInstrucao.destino] = 0;
                    }
                    break;
                case 5:
                    if(vetorDeReg[conjuntoInstrucao.ope1] != vetorDeReg[conjuntoInstrucao.ope2]){
                        vetorDeReg[conjuntoInstrucao.destino] = 1;
                    } else {
                        vetorDeReg[conjuntoInstrucao.destino] = 0;
                    }
                    break;
                case 6:
                    if(vetorDeReg[conjuntoInstrucao.ope1] < vetorDeReg[conjuntoInstrucao.ope2]){
                        vetorDeReg[conjuntoInstrucao.destino] = 1;
                    } else {
                        vetorDeReg[conjuntoInstrucao.destino] = 0;
                    }
                    break;
                case 7:
                    if(vetorDeReg[conjuntoInstrucao.ope1] > vetorDeReg[conjuntoInstrucao.ope2]){
                        vetorDeReg[conjuntoInstrucao.destino] = 1;
                    } else {
                        vetorDeReg[conjuntoInstrucao.destino] = 0;
                    }
                    break;
                case 8:
                    if(vetorDeReg[conjuntoInstrucao.ope1] <= vetorDeReg[conjuntoInstrucao.ope2]){
                        vetorDeReg[conjuntoInstrucao.destino] = 1;
                    } else {
                        vetorDeReg[conjuntoInstrucao.destino] = 0;
                    }
                    break;
                case 9:
                    if(vetorDeReg[conjuntoInstrucao.ope1] >= vetorDeReg[conjuntoInstrucao.ope2]){
                        vetorDeReg[conjuntoInstrucao.destino] = 1;
                    } else {
                        vetorDeReg[conjuntoInstrucao.destino] = 0;
                    }
                    break;
                case 10:
                    vetorDeReg[conjuntoInstrucao.destino] = vetorDeReg[conjuntoInstrucao.ope1] && vetorDeReg[conjuntoInstrucao.ope2];
                    break;
                case 11:
                    vetorDeReg[conjuntoInstrucao.destino] = vetorDeReg[conjuntoInstrucao.ope1] || vetorDeReg[conjuntoInstrucao.ope2];
                    break;
                case 12:
                    vetorDeReg[conjuntoInstrucao.destino] = vetorDeReg[conjuntoInstrucao.ope1] ^ vetorDeReg[conjuntoInstrucao.ope2];
                    break;
                case 13:
                    vetorDeReg[conjuntoInstrucao.destino] = vetorDeReg[conjuntoInstrucao.ope1] << vetorDeReg[conjuntoInstrucao.ope2];
                    break;
                case 14:
                    vetorDeReg[conjuntoInstrucao.destino] = vetorDeReg[conjuntoInstrucao.ope1] >> vetorDeReg[conjuntoInstrucao.ope2];
                    break;
                case 15:
                    int endLoadRegOpe1 = vetorDeReg[conjuntoInstrucao.ope1];
                    vetorDeReg[conjuntoInstrucao.destino] = memoria[endLoadRegOpe1]; 
                    break;
                case 16:
                    int endStoreRegOpe1 = vetorDeReg[conjuntoInstrucao.ope1];
                    memoria[endStoreRegOpe1] = vetorDeReg[conjuntoInstrucao.ope2];
                    break;
                case 63:
                    if (vetorDeReg[0] == 0){
                        estaRodando = 0;
                    }
                    break;
            }
            break;
        case 1:
            switch (conjuntoInstrucao.opcodeI){
                case 0:
                    pc = conjuntoInstrucao.imediato;
                    vaiPular = 0;
                    pulou = 1;
                    break;
                case 1:
                    if(vetorDeReg[conjuntoInstrucao.registrador] == 1){
                        pc = conjuntoInstrucao.imediato;
                        vaiPular = 0;
                        pulou = 1;
                    }
                    break;
                case 3:
                    vetorDeReg[conjuntoInstrucao.registrador] = conjuntoInstrucao.imediato;
                    break;
            }
            break;
    }
    atualizaPreditor(pcExecucao, pulou);
}

void pipeline(uint16_t *memoria) {
    busca(memoria, pc);

    if (pcDecodificacao != -1) {
        execucaoInstrucao = decodificacao(buscaInstrucao);
        pcExecucao = pcDecodificacao;
        pcDecodificacao = -1;
    }

    if (pcExecucao != -1) {
        execucao(memoria, execucaoInstrucao);
        pcExecucao = -1;
    }

    pcDecodificacao = pc;

    if (vaiPular) {
        pc++;
    }
}

int main(int argc, char **argv) {
    if (argc != 2) {
        printf("usage: %s [bin_name]\n", argv[0]);
        exit(1);
    }

    uint16_t memoria[TAMANHO_DE_MEMORIA] = {0};
    load_binary_to_memory(argv[1], memoria, TAMANHO_DE_MEMORIA);

    srand(time(NULL));

    while (estaRodando) {
        pipeline(memoria);

        if (pc >= TAMANHO_DE_MEMORIA) {
            estaRodando = 0;
        }
    }

    printf("----------------------\n");
    printf("Registradores\n");
    for (int i = 0; i < 8; i++) {
        printf("Registrador %d: %d\n", i, vetorDeReg[i]);
    }
    printf("----------------------\n");
    for (int i = 0; i <= 100; i++) {
        printf("%d ", memoria[i]);
    }
    printf("\n");
    return 0;
}