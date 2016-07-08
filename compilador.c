#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>

#define MAXCAD 32

typedef enum {
	INICIO, FIN, LEER, ESCRIBIR, ID,
	CONSTANTE, PARENIZQUIERDO, PARENDERECHO, PUNTOYCOMA,
	COMA, ASIGNACION, SUMA, RESTA, FDT, ERRORLEXICO
} TOKEN;

typedef struct tablaSimbolos {
	char nombre[MAXCAD + 1];
	TOKEN tipo;
	int valor;
} TS;

FILE *in;
FILE *out;
char buffer[MAXCAD + 1];
int estado = 0;
TOKEN tokenActual;
TS tokenReservado[50] = {
		{"inicio", INICIO},
		{"fin", FIN},
		{"leer", LEER},
		{"escribir", ESCRIBIR},
		{"$",}
};
int flagToken = 0;
char operacion = '\0';
int numeroTemp = 1;

void objetivo(void);
void programa(void);
void match(TOKEN token);
TOKEN proximoToken(void);
TOKEN scanner(void);
int estadoFinal(void);
int columna(char c);
void listaSentencias(void);
void sentencia(void);
void primaria(TS *val);
void expresion(TS *resultado);
void listaExpresiones(void);
void operadorAditivo(char *s);
void listaIdentificadores(void);
void errorSintactico();
TOKEN tipoID(void);
void limpiarBuffer(void);
int esIgual(char *s1, char *s2);


void copiar(char *s1, char *s2);
void chequear(char *s);
int buscar(char *s);
void colocar(char *s);
void generar(char *s1, char *s2, char *s3, char *s4);
char * extraer(TS * val);
void identificador(TS *id);
void terminar(void);
void comenzar(void);
TS procesarId(void);
void asignar(TS tIzq, TS tDer);
TS procesarCte(void);
void leer(TS in);
void escribir(TS out);


int main(int argc, char *argv[]) {

	/* ---------- Validación ejecución ---------- */
    if(argc == 1) {
    	printf("> Debe ingresar argumentos.\n> Ejemplo: ./compilador fuente.m salida.o\n");
    	return -1;
    }

    if(argc != 3) {
    	printf("> Número incorrecto de argumentos.\n> Ejemplo: ./compilador fuente.m salida.o\n");
    	return -2;
    }

    /* Verifico extensión archivo entrada: */
    size_t i = 0;
    while(argv[1][i]) i++;
    if( (argv[1][i-2] != '.') || (argv[1][i-1]) != 'm' ) {
    	printf("> El primer argumento debe ser un archivo de extension \".m\".\n> Ejemplo: ./compilador fuente.m salida.o\n");
    	return -3;
   	}

    if( (in=fopen(argv[1],"r")) == NULL ) {
    	printf("> No se pudo abrir el archivo \"%s\".\n", argv[1]);
    	return -4;
    }

    if( (out=fopen(argv[2],"w")) == NULL ) {
        printf("> No se pudo abrir el archivo \"%s\".\n", argv[2]);
        fclose(in);
        return -5;
    }
    /* ---------- Validaciones ejecución ---------- */


   	objetivo();

   	fclose(in);
   	fclose(out);
    return 0;
}


/* -------------------- Rutinas síntesis --------------------- */

void comenzar(void) {
	/* Inicializaciones Semanticas */
	printf("\n# Se inicia rutina de Compilación #\n\n");
}


void terminar(void) {
	generar("Detiene", "", "", "");
}

TS procesarId(void) {
	TS t;
	chequear(buffer);
	t.tipo = ID;
	copiar(buffer, t.nombre);
	return t;
}

char * procesarop(void)
{
	return buffer;
}

void asignar(TS tIzq, TS tDer) {
	generar("Almacena", extraer(&tDer), extraer(&tIzq), "");
}

void leer(TS in) {
	generar("Read", extraer(&in), "Entera", "");
}

TS procesarCte(void) {
	TS t;
	t.tipo = CONSTANTE;
	copiar(buffer, t.nombre);
	sscanf(buffer, "%d", &t.valor);
	return t;
}

void escribir(TS out) {
	generar("Write", extraer(&out), "Entera", "");
}

TS genInfijo(TS val1, char operacion, TS val2 ) {

	TS temp;
	sprintf(temp.nombre,"Temp&%d",numeroTemp); /* hace lo de variables temporales */
	chequear(temp.nombre);

	if(operacion == '+') {
		generar("Suma", extraer(&val1), extraer(&val2), extraer(&temp));
	}

	if(operacion == '-') {
		generar("Resta", extraer(&val1), extraer(&val2), extraer(&temp));
	}

	numeroTemp++;
	return temp;
}



/* -------------------- Funciones extra -------------------- */

void identificador(TS *id)

{
	/* <identificador> -> ID #procesar_id */
	match(ID);
	*id = procesarId();
}

char * extraer(TS *val) {
	return val->nombre;
}

void copiar(char *s1, char *s2) {

	int i = 0;

	while(s1[i] != '\0') {
		s2[i] = s1[i];
		i++;
	}
	s2[i] = '\0';
}

void chequear(char *s) {

	if(!buscar(s)) {
		colocar(s);
		generar("Declara", s, "Entera", "");
	}
}

int buscar(char *s) {

	int i = 0;

	while(tokenReservado[i].nombre[0] != '$') {
		if( esIgual(buffer, tokenReservado[i].nombre) ) return 1;
		i++;
	}
	return 0;
}

void colocar(char *s) {
	int j = 0;
	int i = 0;

	while(tokenReservado[i].nombre[0] != '$') i++;

	tokenReservado[i+1].nombre[0] = '$';

	while(buffer[j] != '\0') {
		tokenReservado[i].nombre[j] = buffer[j];
		j++;
	}
	tokenReservado[i].nombre[j] = '\0';
	tokenReservado[i].tipo = ID;
}

void generar(char* cad1, char* cad2, char* cad3, char* cad4)
{
    fprintf(out,"%s",cad1);
    if(!esIgual("",cad2))    fprintf(out," %s",cad2);
    if(!esIgual("",cad3))    fprintf(out,", %s",cad3);
    if(!esIgual("",cad4))    fprintf(out,", %s",cad4);
    fprintf(out,"\n");
    /* PARA MOSTRAR POR PANTALLA */
    printf("> %s",cad1);
    if(!esIgual("",cad2))    printf(" %s",cad2);
    if(!esIgual("",cad3))    printf(", %s",cad3);
    if(!esIgual("",cad4))    printf(", %s",cad4);
    printf("\n");
    return;
}

int esIgual(char *s1, char *s2) { /* Retorna 1 cuando son iguales */

	int i = 0;

	while( (s1[i] != '\0') && (s2[i] != '\0') ) {
		if(s1[i] != s2[i]) return 0;
		i++;
	}

	if( (s1[i] != '\0') || (s2[i] != '\0') ) return 0;

	return 1;
}

void limpiarBuffer(void) {
	int i;
	for(i = 0; i < (MAXCAD + 1); i++) buffer[i] = '\0';
}

TOKEN tipoID(void) {

	int i = 0;

	while(tokenReservado[i].nombre[0] != '$') {
		if( esIgual(buffer, tokenReservado[i].nombre) ) return tokenReservado[i].tipo;
		i++;
	}
	return ID;
}

void errorLexico() {
	printf("> Error Lexico\n");
	fclose(in);
	fclose(out);
	exit(-1);
}

void errorSintactico() {
	printf("> Error Sintactico\n");
	fclose(in);
	fclose(out);
	exit(-1);
}

/* ------------------- Funciones principales -------------------- */

void match(TOKEN token) {
	proximoToken();

	if (token != proximoToken())
		errorSintactico();

	flagToken = 0;
}

TOKEN proximoToken(void) {
	TOKEN token;

	if(!flagToken) {
		token = scanner();
		if(token == ERRORLEXICO)
			errorLexico();

		if((token == ID)) token = tipoID();

		tokenActual = token;
		flagToken = 1;
	}

	return tokenActual;
}


TOKEN scanner(void) {

	char c;
	int numCol;
	int i = 0;
	estado = 0;

	int tabla[15][13] = {
			/* TT   L   D   +   -   (   )   ,   ;   :   =   FDT SP otro*/
			/* 0*/{ 1 , 3 , 5 , 6 , 7 , 8 , 9 , 10, 11, 14, 13, 0 , 14 },
			/* 1*/{ 1 , 1 , 2 , 2 , 2 , 2 , 2 , 2 , 2 , 2 , 2 , 2 , 2  },
			/* 2*/{ 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14 },
			/* 3*/{ 4 , 3 , 4 , 4 , 4 , 4 , 4 , 4 , 4 , 4 , 4 , 4 , 4  },
			/* 4*/{ 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14 },
			/* 5*/{ 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14 },
			/* 6*/{ 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14 },
			/* 7*/{ 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14 },
			/* 8*/{ 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14 },
			/* 9*/{ 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14 },
			/*10*/{ 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14 },
			/*11*/{ 14, 14, 14, 14, 14, 14, 14, 14, 14, 12, 14, 14, 14 },
			/*12*/{ 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14 },
			/*13*/{ 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14 },
			/*14*/{ 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14 }
	};

	limpiarBuffer();

	do {
		c = fgetc(in);
		numCol = columna(c);
		estado = tabla[estado][numCol];
		if(numCol != 11) {
			buffer[i] = c;
			i++;
		}
	} while( !estadoFinal() && (estado != 14) );

	buffer[i] = '\0';
	switch(estado) {

		case 2: {
			if (numCol != 11) {
				ungetc(c, in);
				buffer[i-1] = '\0';
			}
			return ID;
		}

		case 4:
			if (numCol != 11) {
			ungetc(c, in);
			buffer[i-1] = '\0';
		} return CONSTANTE;

		case 5: return SUMA;

		case 6: return RESTA;

		case 7: return PARENIZQUIERDO;

		case 8: return PARENDERECHO;

		case 9: return COMA;

		case 10: return PUNTOYCOMA;

		case 12: return ASIGNACION;

		case 13: return FDT;

		case 14: return ERRORLEXICO;
	}

	return -1;
}


int columna(char c) {

	if(isalpha(c)) return 0;
	if(isdigit(c)) return 1;
	if(c == '+') return 2;
	if(c == '-') return 3;
	if(c == '(') return 4;
	if(c == ')') return 5;
	if(c == ',') return 6;
	if(c == ';') return 7;
	if(c == ':') return 8;
	if(c == '=') return 9;
	if(c == EOF) return 10;
	if(isspace(c)) return 11;
	return 12;
}


int estadoFinal() {
	if( (estado == 0) || (estado == 1) || (estado == 3) || (estado == 11) ) return 0;
	else return 1;
}

/* ---------- PAS ---------- */

void objetivo(void) {
	/* <objetivo> -> <programa> FDT #terminar */
	programa();
	match(FDT);
	terminar();
}

void programa(void) {
	/* <programa> -> #comenzar INICIO <listaSentencias> FIN */
	comenzar();
	match(INICIO);
	listaSentencias();
	match(FIN);
}

void listaSentencias(void) {
	/* <listaSentencias> -> <sentencia> {<sentencia>} */
	sentencia();


	while(1) {
		proximoToken();

		switch(tokenActual) {
			case ID: {
				sentencia();
				break;
			}
			case LEER: {
				sentencia();
				break;
			}
			case ESCRIBIR: {
				sentencia();
				break;
			}
			default: return;
		}
	}
}

void sentencia(void) {
	TS izq,der;

	proximoToken();

	switch(tokenActual) {

		case ID: {
			/* <sentencia> -> ID := <expresion> #asignar ; */
			identificador(&izq);
			match(ASIGNACION);
			expresion(&der);
			asignar(izq, der);
			match(PUNTOYCOMA);
			break;
		}

		case LEER: {
			/* <sentencia> -> LEER ( <listaIdentificadores> ) */
			match(LEER);
			match(PARENIZQUIERDO);
			listaIdentificadores();
			match(PARENDERECHO);
			match(PUNTOYCOMA);
			break;
		}

		case ESCRIBIR: {
			/* <sentencia> -> ESCRIBIR ( <listaExpresiones> ) */
			match(ESCRIBIR);
			match(PARENIZQUIERDO);
			listaExpresiones();
			match(PARENDERECHO);
			match(PUNTOYCOMA);
			break;
		}

		default: {
			errorSintactico();
			break;
		}
	}
}

void expresion(TS *resultado) {
	/* <expresion> -> <primaria> { <operadorAditivo> <primaria> #gen_infijo } */
	TS opizq;
	TS opder;
	char op;
	primaria(&opizq);

	proximoToken();

	while( (tokenActual == SUMA) || (tokenActual == RESTA) ) {
		operadorAditivo(&op);
		primaria(&opder);
		opizq = genInfijo(opizq,op,opder);
		proximoToken();
	}
	*resultado = opizq;
}

void primaria(TS *val) {
	switch(proximoToken()) {

		case ID: {
			/* <primaria> -> <identificador> */
			identificador(val);
			break;
		}
		case CONSTANTE: {
			/* <primaria> -> CONSTANTE #procesar_cte */
			match(CONSTANTE);
			*val = procesarCte();
			break;
		}
		case PARENIZQUIERDO: {
			/* <primaria> -> PARENIZQUIERDO <expresion> PARENDERECHO */
			match(PARENIZQUIERDO);
			expresion(val);
			match(PARENDERECHO);
			break;
		}
		default: errorSintactico();
	}
}

void operadorAditivo(char *op) {
	/* <operadorAditivo> -> SUMA #procesar_op | RESTA #procesar_op */
	TOKEN token = proximoToken();

	if( (token == SUMA) || (token == RESTA) ){
		match(token);
		copiar(procesarop(), op);
	}
	else errorSintactico();
}

void listaExpresiones() {
	/* <listaExpresiones> -> <expresion> #escribir_exp {COMA <expresion> #escribir_exp} */
	TS reg;

	expresion(&reg);

	escribir(reg);

	while(1) {
		proximoToken();
		if(tokenActual == COMA) {
			match(COMA);
			expresion(&reg);
			escribir(reg);
			proximoToken();
		} else return;
	}
}

void listaIdentificadores() {
	/* <listaIdentificadores> -> <identificador> #leer_id {COMA <identificador> #leer_id} */
	TS reg;

	identificador(&reg);
	leer(reg);

	while(1) {
		proximoToken();
		if(tokenActual == COMA) {
			match(COMA);
			identificador(&reg);
			leer(reg);
			proximoToken();
		} else return;
	}
}
