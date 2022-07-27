# Montador-Ligador

Primeiro trabalho de Software Básico. Montador e Ligador para um assembly fictício.

Alunos:

- 19/0047089 - Lucas de Almeida Bandeira Macedo
- 19/0015292 - João Pedro Felix de Almeida

Semestre 2022.1

## Como compilar

```bash
g++ montador.cpp -o montador.out -std=c++17
g++ ligador.cpp -o ligador.out
```

Os programas foram feitos em Linux e, já que escrevem em arquivos durante a execução, precisam estar em pastas com as permissões necessárias.

## Como usar

### Montador

```bash
./montador -p codigo.asm codigo_pre_processado.obj
./montador -o codigo.asm objeto.asm
```

Para pré-processamento e geração de código objeto, respectivamente

### Ligador

```bash
./ligador myfile1.o myfile2.o
```

O output do ligador será salvo como `./output.bin`

## Observações

MULT e JNP não são reconhecidos pelo programa. Por favor, use MUL e JMP.
