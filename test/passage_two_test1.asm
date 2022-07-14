; conta de salto em salto ate que a contagem seja igual ou superior ao limite
ZERO: EQU 0

SECAO TEXTO
load inicio
store contador

input salto
input limite
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
salto: space
inicio: const zEro
limite: space