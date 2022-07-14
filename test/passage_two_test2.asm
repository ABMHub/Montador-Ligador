; conta de salto em salto ate que a contagem seja igual ou superior ao limite
ZERO: EQU 0

SECAO TEXTO
load inicio
store contador

loop: 
    output contador
    load contador
    add salto
    store contador

    sub limite
    jmpn loop
    jmpz loop

stop

SECAO dados
contador: space
salto: const 0x10
inicio: const -64
limite: const 64