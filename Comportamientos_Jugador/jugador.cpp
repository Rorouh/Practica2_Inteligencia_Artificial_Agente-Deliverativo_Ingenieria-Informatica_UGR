#include "../Comportamientos_Jugador/jugador.hpp"
#include "motorlib/util.h"

#include <iostream>
#include <cmath>
#include <set>
#include <stack>
#include <random>

// Este es el método principal que se piden en la practica.
// Tiene como entrada la información de los sensores y devuelve la acción a realizar.
// Para ver los distintos sensores mirar fichero "comportamiento.hpp"
Action ComportamientoJugador::think(Sensores sensores)
{
	Action accion = actIDLE;
	//ACTUALIZAR LAS VARIABLES
	if(sensores.nivel == 4){
		if(principio){
			accion = actWHEREIS;
			principio = false;
			return accion;
		}else{
			ActualizaBrujula(fila,columna,orientacion,accion);
			actual.fila =fila;
			actual.columna = columna;
			actual.orientacion = orientacion;
		}
	}else{
		actual.fila = sensores.posF;
		actual.columna = sensores.posC;
		actual.orientacion = sensores.sentido;
		cout << "Fila: " << actual.fila << endl;
		cout << "Col : " << actual.columna << endl;
		cout << "Ori : " << actual.orientacion << endl;
	}
	
	
	
	// Capturo los destinos
	cout << "sensores.num_destinos : " << sensores.num_destinos << endl;
	objetivos.clear();
	for (int i = 0; i < sensores.num_destinos; i++)
	{
		estado aux;
		aux.fila = sensores.destino[2 * i];
		aux.columna = sensores.destino[2 * i + 1];
		objetivos.push_back(aux);
	}
    
	//si no hay plan,construyo uno
	if(!hayPlan ){
		if(sensores.nivel == 3 or sensores.nivel == 4) InicioJuego(sensores);
		hayPlan = pathFinding(sensores.nivel,actual,objetivos, plan,sensores);
	}
	Action sigAccion;
    
	if(sensores.nivel == 3 or sensores.nivel == 4){
		if(plan.empty()){
			hayPlan = pathFinding(sensores.nivel,actual,objetivos, plan,sensores);
		}
	}

	if(hayPlan and plan.size()>0){ //hay un plan no vacio
		if(sensores.nivel == 3 or sensores.nivel == 4){
			Vision(sensores);
			CompruebaPlan(sensores,plan);
		}
		sigAccion = plan.front(); // tomo la siguiente accion del plan
		plan.erase(plan.begin()); // eliminamos la accion del plan
	}else{
		cout << "no se pudo encontrar plan";
	}
    if(plan.size()==0 and objetivos.empty())
	return actIDLE;
	// bool hay_plan = pathFinding(sensores.nivel, actual, objetivos, plan);
	// return accion;
	
	return sigAccion;
}

// Llama al algoritmo de busqueda que se usara en cada comportamiento del agente
// Level representa el comportamiento en el que fue iniciado el agente.
bool ComportamientoJugador::pathFinding(int level, const estado &origen, const list<estado> &destino, list<Action> &plan,Sensores sensores)
{
	switch (level)
	{
	case 0:
		cout << "Demo\n";
		estado un_objetivo;
		un_objetivo = objetivos.front();
		cout << "fila: " << un_objetivo.fila << " col:" << un_objetivo.columna << endl;
		return pathFinding_Profundidad(origen, un_objetivo, plan);
		break;

	case 1:
		cout << "Optimo numero de acciones\n";
		// Incluir aqui la llamada al busqueda en anchura
		un_objetivo = objetivos.front();
		cout << "fila: " << un_objetivo.fila << " col: " << un_objetivo.columna << endl;
		return pathFinding_anchura(origen,un_objetivo,plan);
		break;
	case 2:
		cout << "Optimo numero de acciones\n";
		// Incluir aqui la llamada al busqueda en anchura
		un_objetivo = objetivos.front();
		cout << "fila: " << un_objetivo.fila << " col: " << un_objetivo.columna << endl;
		return pathFinding_costeUniforme(origen,un_objetivo,plan,sensores);
		break;
	case 3:
		cout << "Reto 1: Descubrir el mapa\n";
		// Incluir aqui la llamada al algoritmo de busqueda para el Reto 1 ACTUALIZAR VISION ANTES
		estado objetivo;
		buscaCasilla(objetivo);
		if(objetivo.columna==-1)
			return false;
        return pathFinding_costeUniforme(origen,objetivo,plan,sensores);
		break;
	case 4:
		cout << "Reto 2: Maximizar objetivos\n";
		// Incluir aqui la llamada al algoritmo de busqueda para el Reto 2
		cout << "No implementado aun\n";
		break;
	}
	return false;
}

//---------------------- Implementación de la busqueda en profundidad ---------------------------

// Dado el codigo en caracter de una casilla del mapa dice si se puede
// pasar por ella sin riegos de morir o chocar.
bool EsObstaculo(unsigned char casilla)
{
	if (casilla == 'P' or casilla == 'M')
		return true;
	else
		return false;
}

// Comprueba si la casilla que hay delante es un obstaculo. Si es un
// obstaculo devuelve true. Si no es un obstaculo, devuelve false y
// modifica st con la posición de la casilla del avance.
bool ComportamientoJugador::HayObstaculoDelante(estado &st)
{
	int fil = st.fila, col = st.columna;

	// calculo cual es la casilla de delante del agente
	switch (st.orientacion)
	{
	case 0:
		fil--;
		break;
	case 1:
		fil--;
		col++;
		break;
	case 2:
		col++;
		break;
	case 3:
		fil++;
		col++;
		break;
	case 4:
		fil++;
		break;
	case 5:
		fil++;
		col--;
		break;
	case 6:
		col--;
		break;
	case 7:
		fil--;
		col--;
		break;
	}

	// Compruebo que no me salgo fuera del rango del mapa
	if (fil < 0 or fil >= mapaResultado.size())
		return true;
	if (col < 0 or col >= mapaResultado[0].size())
		return true;

	// Miro si en esa casilla hay un obstaculo infranqueable
	if (!EsObstaculo(mapaResultado[fil][col]))
	{
		// No hay obstaculo, actualizo el parametro st poniendo la casilla de delante.
		st.fila = fil;
		st.columna = col;
		return false;
	}
	else
	{
		return true;
	}
}

struct nodo
{
	estado st;
	list<Action> secuencia;
};

struct ComparaEstados
{
	bool operator()(const estado &a, const estado &n) const
	{
		if ((a.fila > n.fila) or (a.fila == n.fila and a.columna > n.columna) or
			(a.fila == n.fila and a.columna == n.columna and a.orientacion > n.orientacion) or
			(a.fila == n.fila and a.columna == n.columna and a.orientacion == n.orientacion and a.bikini_aux > n.bikini_aux ) or
			(a.fila == n.fila and a.columna == n.columna and a.orientacion == n.orientacion and a.bikini_aux == n.bikini_aux and a.zapatillas_aux > n.zapatillas_aux)) 
			return true;
		else
			return false;
	}
};

// Implementación de la busqueda en profundidad.
// Entran los puntos origen y destino y devuelve la
// secuencia de acciones en plan, una lista de acciones.
bool ComportamientoJugador::pathFinding_Profundidad(const estado &origen, const estado &destino, list<Action> &plan)
{
	// Borro la lista
	cout << "Calculando plan\n";
	plan.clear();
	set<estado, ComparaEstados> Cerrados; // Lista de Cerrados
	stack<nodo> Abiertos;				  // Lista de Abiertos

	nodo current;
	current.st = origen;
	current.secuencia.empty();

	Abiertos.push(current);

	while (!Abiertos.empty() and (current.st.fila != destino.fila or current.st.columna != destino.columna))
	{

		Abiertos.pop();
		Cerrados.insert(current.st);

		// Generar descendiente de girar a la derecha 90 grados
		nodo hijoTurnR = current;
		hijoTurnR.st.orientacion = (hijoTurnR.st.orientacion + 2) % 8;
		if (Cerrados.find(hijoTurnR.st) == Cerrados.end())
		{
			hijoTurnR.secuencia.push_back(actTURN_R);
			Abiertos.push(hijoTurnR);
		}

		// Generar descendiente de girar a la derecha 45 grados
		nodo hijoSEMITurnR = current;
		hijoSEMITurnR.st.orientacion = (hijoSEMITurnR.st.orientacion + 1) % 8;
		if (Cerrados.find(hijoSEMITurnR.st) == Cerrados.end())
		{
			hijoSEMITurnR.secuencia.push_back(actSEMITURN_R);
			Abiertos.push(hijoSEMITurnR);
		}

		// Generar descendiente de girar a la izquierda 90 grados
		nodo hijoTurnL = current;
		hijoTurnL.st.orientacion = (hijoTurnL.st.orientacion + 6) % 8;
		if (Cerrados.find(hijoTurnL.st) == Cerrados.end())
		{
			hijoTurnL.secuencia.push_back(actTURN_L);
			Abiertos.push(hijoTurnL);
		}

		// Generar descendiente de girar a la izquierda 45 grados
		nodo hijoSEMITurnL = current;
		hijoSEMITurnL.st.orientacion = (hijoSEMITurnL.st.orientacion + 7) % 8;
		if (Cerrados.find(hijoSEMITurnL.st) == Cerrados.end())
		{
			hijoSEMITurnL.secuencia.push_back(actSEMITURN_L);
			Abiertos.push(hijoSEMITurnL);
		}

		// Generar descendiente de avanzar
		nodo hijoForward = current;
		if (!HayObstaculoDelante(hijoForward.st))
		{
			if (Cerrados.find(hijoForward.st) == Cerrados.end())
			{
				hijoForward.secuencia.push_back(actFORWARD);
				Abiertos.push(hijoForward);
			}
		}

		// Tomo el siguiente valor de la Abiertos
		if (!Abiertos.empty())
		{
			current = Abiertos.top();
		}
	}

	cout << "Terminada la busqueda\n";

	if (current.st.fila == destino.fila and current.st.columna == destino.columna)
	{
		cout << "Cargando el plan\n";
		plan = current.secuencia;
		cout << "Longitud del plan: " << plan.size() << endl;
		PintaPlan(plan);
		// ver el plan en el mapa
		VisualizaPlan(origen, plan);
		return true;
	}
	else
	{
		cout << "No encontrado plan\n";
	}

	return false;
}

// Sacar por la consola la secuencia del plan obtenido
void ComportamientoJugador::PintaPlan(list<Action> plan)
{
	auto it = plan.begin();
	while (it != plan.end())
	{
		if (*it == actFORWARD)
		{
			cout << "A ";
		}
		else if (*it == actTURN_R)
		{
			cout << "D ";
		}
		else if (*it == actSEMITURN_R)
		{
			cout << "d ";
		}
		else if (*it == actTURN_L)
		{
			cout << "I ";
		}
		else if (*it == actSEMITURN_L)
		{
			cout << "i ";
		}
		else
		{
			cout << "- ";
		}
		it++;
	}
	cout << endl;
}

// Funcion auxiliar para poner a 0 todas las casillas de una matriz
void AnularMatriz(vector<vector<unsigned char>> &m)
{
	for (int i = 0; i < m[0].size(); i++)
	{
		for (int j = 0; j < m.size(); j++)
		{
			m[i][j] = 0;
		}
	}
}

// Pinta sobre el mapa del juego el plan obtenido
void ComportamientoJugador::VisualizaPlan(const estado &st, const list<Action> &plan)
{
	AnularMatriz(mapaConPlan);
	estado cst = st;

	auto it = plan.begin();
	while (it != plan.end())
	{
		if (*it == actFORWARD)
		{
			switch (cst.orientacion)
			{
			case 0:
				cst.fila--;
				break;
			case 1:
				cst.fila--;
				cst.columna++;
				break;
			case 2:
				cst.columna++;
				break;
			case 3:
				cst.fila++;
				cst.columna++;
				break;
			case 4:
				cst.fila++;
				break;
			case 5:
				cst.fila++;
				cst.columna--;
				break;
			case 6:
				cst.columna--;
				break;
			case 7:
				cst.fila--;
				cst.columna--;
				break;
			}
			mapaConPlan[cst.fila][cst.columna] = 1;
		}
		else if (*it == actTURN_R)
		{
			cst.orientacion = (cst.orientacion + 2) % 8;
		}
		else if (*it == actSEMITURN_R)
		{
			cst.orientacion = (cst.orientacion + 1) % 8;
		}
		else if (*it == actTURN_L)
		{
			cst.orientacion = (cst.orientacion + 6) % 8;
		}
		else if (*it == actSEMITURN_L)
		{
			cst.orientacion = (cst.orientacion + 7) % 8;
		}
		it++;
	}
}

int ComportamientoJugador::interact(Action accion, int valor)
{
	return false;
}


//Metodo nivel 1, busqueda en anchura
bool ComportamientoJugador::pathFinding_anchura(const estado &origen, const estado &destino, list<Action> &plan) {

	// Borro la lista
	cout << "Calculando plan por anchura\n";
	plan.clear();
	set<estado, ComparaEstados> Cerrados; // Lista de Cerrados
	queue<nodo> Abiertos;				  // Lista de Abiertos

	nodo current;
	current.st = origen;
	current.secuencia.empty();

	Abiertos.push(current);


	while (!Abiertos.empty() and (current.st.fila != destino.fila or current.st.columna != destino.columna))
	{

		Abiertos.pop();
		Cerrados.insert(current.st);

		// Generar descendiente de girar a la derecha 90 grados
		nodo hijoTurnR = current;
		hijoTurnR.st.orientacion = (hijoTurnR.st.orientacion + 2) % 8;
		if (Cerrados.find(hijoTurnR.st) == Cerrados.end())
		{
			hijoTurnR.secuencia.push_back(actTURN_R);
			Abiertos.push(hijoTurnR);
		}

		// Generar descendiente de girar a la derecha 45 grados
		nodo hijoSEMITurnR = current;
		hijoSEMITurnR.st.orientacion = (hijoSEMITurnR.st.orientacion + 1) % 8;
		if (Cerrados.find(hijoSEMITurnR.st) == Cerrados.end())
		{
			hijoSEMITurnR.secuencia.push_back(actSEMITURN_R);
			Abiertos.push(hijoSEMITurnR);
		}

		// Generar descendiente de girar a la izquierda 90 grados
		nodo hijoTurnL = current;
		hijoTurnL.st.orientacion = (hijoTurnL.st.orientacion + 6) % 8;
		if (Cerrados.find(hijoTurnL.st) == Cerrados.end())
		{
			hijoTurnL.secuencia.push_back(actTURN_L);
			Abiertos.push(hijoTurnL);
		}

		// Generar descendiente de girar a la izquierda 45 grados
		nodo hijoSEMITurnL = current;
		hijoSEMITurnL.st.orientacion = (hijoSEMITurnL.st.orientacion + 7) % 8;
		if (Cerrados.find(hijoSEMITurnL.st) == Cerrados.end())
		{
			hijoSEMITurnL.secuencia.push_back(actSEMITURN_L);
			Abiertos.push(hijoSEMITurnL);
		}

		// Generar descendiente de avanzar
		nodo hijoForward = current;
		if (!HayObstaculoDelante(hijoForward.st))
		{
			if (Cerrados.find(hijoForward.st) == Cerrados.end())
			{
				hijoForward.secuencia.push_back(actFORWARD);
				Abiertos.push(hijoForward);
			}
		}

		// --------- Implementacion adicional ------------ para mejorar la eficiencia
		// Comprobacion de que el siguiente nodo no este en cerrados
		while (Cerrados.find(Abiertos.front().st) != Cerrados.end() ){
			Abiertos.pop();
		}
		
		// Tomo el siguiente valor de la Abiertos
		if (!Abiertos.empty())
		{
			current = Abiertos.front();
		}
	}



	cout << "Terminada la busqueda\n";

	if (current.st.fila == destino.fila and current.st.columna == destino.columna)
	{
		cout << "Cargando el plan\n";
		plan = current.secuencia;
		cout << "Longitud del plan: " << plan.size() << endl;
		PintaPlan(plan);
		// ver el plan en el mapa
		VisualizaPlan(origen, plan);
		return true;
	}
	else
	{
		cout << "No encontrado plan\n";
	}


	return false;
}

struct nodo_uniforme{

	estado st;
	list<Action> secuencia;
	int costo;
	bool operator<(const nodo_uniforme &n)const{
		return costo>n.costo;
	}
};

// Metodos del nivel 2, coste uniforme
int ComportamientoJugador::CalculaCoste(estado estado_aux, Action accion){

    switch (accion)
    {
    case actFORWARD:
        if(!estado_aux.bikini_aux and mapaResultado[estado_aux.fila][estado_aux.columna] == 'A'){
            return 200;
        }else if(estado_aux.bikini_aux and mapaResultado[estado_aux.fila][estado_aux.columna] == 'A'){
            return 10;
        }else if(!estado_aux.zapatillas_aux and mapaResultado[estado_aux.fila][estado_aux.columna] == 'B'){
            return 100;
        }else if(estado_aux.zapatillas_aux and mapaResultado[estado_aux.fila][estado_aux.columna] == 'B'){
            return 15;
        }else if(mapaResultado[estado_aux.fila][estado_aux.columna] == 'T'){
            return 2;
		}else{
            return 1;
        }        
        break;
    case actTURN_L:
        if(!estado_aux.bikini_aux and mapaResultado[estado_aux.fila][estado_aux.columna] == 'A'){
            return 500;
        }else if(estado_aux.bikini_aux and mapaResultado[estado_aux.fila][estado_aux.columna] == 'A'){
            return 5;
        }else if(!estado_aux.zapatillas_aux and mapaResultado[estado_aux.fila][estado_aux.columna] == 'B'){
            return 3;
        }else if(estado_aux.zapatillas_aux and mapaResultado[estado_aux.fila][estado_aux.columna] == 'B'){
            return 1;
        }else if(mapaResultado[estado_aux.fila][estado_aux.columna] == 'T'){
            return 2;
		}else{
            return 1;
        }
		break; 
    case actTURN_R:
        if(!estado_aux.bikini_aux and mapaResultado[estado_aux.fila][estado_aux.columna] == 'A'){
            return 500;
        }else if(estado_aux.bikini_aux and mapaResultado[estado_aux.fila][estado_aux.columna] == 'A'){
            return 5;
        }else if(!estado_aux.zapatillas_aux and mapaResultado[estado_aux.fila][estado_aux.columna] == 'B'){
            return 3;
        }else if(estado_aux.zapatillas_aux and mapaResultado[estado_aux.fila][estado_aux.columna] == 'B'){
            return 1;
        }else if(mapaResultado[estado_aux.fila][estado_aux.columna] == 'T'){
            return 2;
		}else{
            return 1;
        }
		break; 
    case actSEMITURN_R:
        if(!estado_aux.bikini_aux and mapaResultado[estado_aux.fila][estado_aux.columna] == 'A'){
            return 300;
        }else if(estado_aux.bikini_aux and mapaResultado[estado_aux.fila][estado_aux.columna] == 'A'){
            return 2;
        }else if(!estado_aux.zapatillas_aux and mapaResultado[estado_aux.fila][estado_aux.columna] == 'B'){
            return 2;
        }else if(estado_aux.zapatillas_aux and mapaResultado[estado_aux.fila][estado_aux.columna] == 'B'){
            return 1;
        }else if(mapaResultado[estado_aux.fila][estado_aux.columna] == 'T'){
            return 1;
		}else{
            return 1;
        }break;
    case actSEMITURN_L:
        if(!estado_aux.bikini_aux and mapaResultado[estado_aux.fila][estado_aux.columna] == 'A'){
            return 300;
        }else if(estado_aux.bikini_aux and mapaResultado[estado_aux.fila][estado_aux.columna] == 'A'){
            return 2;
        }else if(!estado_aux.zapatillas_aux and mapaResultado[estado_aux.fila][estado_aux.columna] == 'B'){
            return 2;
        }else if(estado_aux.zapatillas_aux and mapaResultado[estado_aux.fila][estado_aux.columna] == 'B'){
            return 1;
        }else if(mapaResultado[estado_aux.fila][estado_aux.columna] == 'T'){
            return 1;
        }else{
            return 1;
        }
		break;  
    }
}
bool ComportamientoJugador::pathFinding_costeUniforme(const estado &origen, const estado &destino, list<Action> &plan, Sensores sensores){

	// Borro la lista
	cout << "Calculando plan por coste uniforme\n";
	plan.clear();
	set<estado, ComparaEstados> Cerrados; 	// Lista de Cerrados
	priority_queue<nodo_uniforme> Abiertos;	// Lista de Abiertos

	nodo_uniforme current;
	current.st = origen;
	current.secuencia.empty();
	current.costo = 0;
    
	Abiertos.push(current);
	while (!Abiertos.empty() and (current.st.fila != destino.fila or current.st.columna != destino.columna)){
		Abiertos.pop();
        if(mapaResultado[current.st.fila][current.st.columna] == 'K'){
			current.st.bikini_aux = true;
			current.st.zapatillas_aux =  false;
		}
		if(mapaResultado[current.st.fila][current.st.columna] == 'D'){
			current.st.zapatillas_aux = true;
			current.st.bikini_aux = false;
		}
		Cerrados.insert(current.st);

		// Generar descendiente de girar a la derecha 90 grados
		nodo_uniforme hijoTurnR = current;
		hijoTurnR.st.orientacion = (hijoTurnR.st.orientacion + 2) % 8;
		if (Cerrados.find(hijoTurnR.st) == Cerrados.end())
		{	
			if(mapaResultado[hijoTurnR.st.fila][hijoTurnR.st.columna] == 'K'){
				hijoTurnR.st.bikini_aux = true;
				hijoTurnR.st.zapatillas_aux = false;
			}
			if(mapaResultado[hijoTurnR.st.fila][hijoTurnR.st.columna] == 'D'){
				hijoTurnR.st.zapatillas_aux = true;
				hijoTurnR.st.bikini_aux = false;
			}
			hijoTurnR.costo +=  CalculaCoste(hijoTurnR.st,actTURN_R);
			hijoTurnR.secuencia.push_back(actTURN_R);
			Abiertos.push(hijoTurnR);
		}

		// Generar descendiente de girar a la derecha 45 grados
		nodo_uniforme hijoSEMITurnR = current;
		hijoSEMITurnR.st.orientacion = (hijoSEMITurnR.st.orientacion + 1) % 8;
		if (Cerrados.find(hijoSEMITurnR.st) == Cerrados.end())
		{
			if(mapaResultado[hijoSEMITurnR.st.fila][hijoSEMITurnR.st.columna] == 'K'){
				hijoSEMITurnR.st.bikini_aux =  true;
				hijoSEMITurnR.st.zapatillas_aux =  false;
			}
			if(mapaResultado[hijoSEMITurnR.st.fila][hijoSEMITurnR.st.columna] == 'D'){
				hijoSEMITurnR.st.zapatillas_aux =true;
				hijoSEMITurnR.st.bikini_aux =  false;
			}
            hijoSEMITurnR.costo += CalculaCoste(hijoSEMITurnR.st,actSEMITURN_R);
			hijoSEMITurnR.secuencia.push_back(actSEMITURN_R);
			Abiertos.push(hijoSEMITurnR);
		}

		// Generar descendiente de girar a la izquierda 90 grados
		nodo_uniforme hijoTurnL = current;
		hijoTurnL.st.orientacion = (hijoTurnL.st.orientacion + 6) % 8;
		if (Cerrados.find(hijoTurnL.st) == Cerrados.end())
		{
			if(mapaResultado[hijoTurnL.st.fila][hijoTurnL.st.columna] == 'K'){
				hijoTurnL.st.bikini_aux =true;
				hijoTurnL.st.zapatillas_aux = false;
			}
			if(mapaResultado[hijoTurnL.st.fila][hijoTurnL.st.columna] == 'D'){
				hijoTurnL.st.zapatillas_aux =  true;
				hijoTurnL.st.bikini_aux = false;
			}
            hijoTurnL.costo += CalculaCoste(hijoTurnL.st,actTURN_L);
			hijoTurnL.secuencia.push_back(actTURN_L);
			Abiertos.push(hijoTurnL);
		}

		// Generar descendiente de girar a la izquierda 45 grados
		nodo_uniforme hijoSEMITurnL = current;
		hijoSEMITurnL.st.orientacion = (hijoSEMITurnL.st.orientacion + 7) % 8;
		if (Cerrados.find(hijoSEMITurnL.st) == Cerrados.end())
		{
			if(mapaResultado[hijoSEMITurnL.st.fila][hijoSEMITurnL.st.columna] == 'K'){
				hijoSEMITurnL.st.bikini_aux = true;
				hijoSEMITurnL.st.zapatillas_aux = false;
			}
			if(mapaResultado[hijoSEMITurnL.st.fila][hijoSEMITurnL.st.columna] == 'D'){
				hijoSEMITurnL.st.zapatillas_aux =  true;
				hijoSEMITurnL.st.bikini_aux = false;
			}
			hijoSEMITurnL.costo += CalculaCoste(hijoSEMITurnL.st,actSEMITURN_L);
            hijoSEMITurnL.secuencia.push_back(actSEMITURN_L);
			Abiertos.push(hijoSEMITurnL);
		}

        
		// Generar descendiente de avanzar
		nodo_uniforme hijoForward = current;
		if (!HayObstaculoDelante(hijoForward.st))
		{
			if (Cerrados.find(hijoForward.st) == Cerrados.end())
			{
				if(mapaResultado[hijoForward.st.fila][hijoForward.st.columna] == 'K'){
					hijoForward.st.bikini_aux =  true;
					hijoForward.st.zapatillas_aux =  false;
				}
				if(mapaResultado[hijoForward.st.fila][hijoForward.st.columna] == 'D'){
					hijoForward.st.zapatillas_aux =true;
					hijoForward.st.bikini_aux = false;
				}
                hijoForward.costo += CalculaCoste(hijoForward.st,actFORWARD);
				hijoForward.secuencia.push_back(actFORWARD);
				Abiertos.push(hijoForward);
			}
		}

        // --------- Implementacion adicional ------------ para mejorar la eficiencia
		// Comprobacion de que el siguiente nodo no este en cerrados
		
        while (Cerrados.find(Abiertos.top().st) != Cerrados.end() ){
			Abiertos.pop();
		}

        // Tomo el siguiente valor de la Abiertos
		if (!Abiertos.empty())
		{
			current = Abiertos.top();
		}

	}

	cout << "Terminada la busqueda\n";

	if (current.st.fila == destino.fila and current.st.columna == destino.columna)
	{
		cout << "Cargando el plan\n";
		plan = current.secuencia;
		cout << "Longitud del plan: " << plan.size() << endl;
		PintaPlan(plan);
		// ver el plan en el mapa
		VisualizaPlan(origen, plan);
		return true;
	}
	else
	{
		cout << "No encontrado plan\n";
	}


	return false;
	

}
// Metodos del nivel 3, descubrir mapa
void ComportamientoJugador::Vision(Sensores sensores){

    int fila = sensores.posF;
    int columna = sensores.posC;
    int col_aux,fil_aux,pos=1;
    int brujula = sensores.sentido;
    mapaResultado[fila][columna] = sensores.terreno[0];
    for(int j=0;j < 4 ; j++){

        if (brujula == 0){
            
            if(pos >=1 and pos <=3){
                col_aux = -1;
                fil_aux = -1;
                for(int i=1; i <= 3 ; i++){
                    mapaResultado[fila+fil_aux][columna+col_aux] = sensores.terreno[i];
                    col_aux++;
                    pos++;
                }
            }else if(pos >=4 and pos <= 8){
                col_aux = -2;
                fil_aux = -2;
                for(int i=4; i <= 8; i++){
                    mapaResultado[fila+fil_aux][columna+col_aux] = sensores.terreno[i];
                    col_aux++;
                    pos++;
                }
            }else if(pos >=9){
                col_aux = -3;
                fil_aux = -3;
                for(int i=9; i <= 15 ; i++){
                    mapaResultado[fila+fil_aux][columna+col_aux] = sensores.terreno[i];
                    col_aux++;
                    pos++;
                }
            }
        }else if( brujula ==2 ){
            if(pos >=1 and pos <=3){
                col_aux = 1;
                fil_aux = -1;
                for(int i=1; i <= 3 ; i++){
                    mapaResultado[fila+fil_aux][columna+col_aux] = sensores.terreno[i];
                    fil_aux++;
                    pos++;
                }
            }else if(pos >=4 and pos <= 8){
                col_aux = 2;
                fil_aux = -2;
                for(int i=4; i <= 8; i++){
                    mapaResultado[fila+fil_aux][columna+col_aux] = sensores.terreno[i];
                    fil_aux++;
                    pos++;
                }
            }else if(pos >=9){
                col_aux = 3;
                fil_aux = -3;
                for(int i=9; i <= 15 ; i++){
                    mapaResultado[fila+fil_aux][columna+col_aux] = sensores.terreno[i];
                    fil_aux++;
                    pos++;
                }
            }
        }else if( brujula ==4 ){
            if(pos >= 1 and pos <=3){
                col_aux = 1;
                fil_aux = 1;
                for(int i=1; i <= 3 ; i++){
                    mapaResultado[fila+fil_aux][columna+col_aux] = sensores.terreno[i];
                    col_aux--;
                    pos++;
                }
            }else if(pos >=4 and pos <= 8){
                col_aux = 2;
                fil_aux = 2;
                for(int i=4; i <= 8; i++){
                    mapaResultado[fila+fil_aux][columna+col_aux] = sensores.terreno[i];
                    col_aux--;
                    pos++;
                }
            }else if(pos >=9){
                col_aux = 3;
                fil_aux = 3;
                for(int i=9; i <= 15 ; i++){
                    mapaResultado[fila+fil_aux][columna+col_aux] = sensores.terreno[i];
                    col_aux--;
                    pos++;
                }
            }
        }else if( brujula ==6 ){
            if(pos >=1 and pos <=3){
                col_aux = -1;
                fil_aux = 1;
                for(int i=1; i <= 3 ; i++){
                    mapaResultado[fila+fil_aux][columna+col_aux] = sensores.terreno[i];
                    fil_aux--;
                    pos++;
                }
            }else if(pos >=4 and pos <= 8){
                col_aux = -2;
                fil_aux = 2;
                for(int i=4; i <= 8; i++){
                    mapaResultado[fila+fil_aux][columna+col_aux] = sensores.terreno[i];
                    fil_aux--;
                    pos++;
                }
            }else if(pos >=9){
                col_aux = -3;
                fil_aux = 3;
                for(int i=9; i <= 15 ; i++){
                    mapaResultado[fila+fil_aux][columna+col_aux] = sensores.terreno[i];
                    fil_aux--;
                    pos++;
                }
            }
        }else if( brujula == 1){
            if( pos >= 1 and pos <= 3){
                fil_aux = -1;
                col_aux = 0;
                for(int i=1;i<=2;i++){
                    mapaResultado[fila+fil_aux][columna+col_aux] = sensores.terreno[i]; 
                    col_aux++;
					pos++;
                }
                mapaResultado[fila][columna+1] = sensores.terreno[3];
            }
            if( pos >= 4 and pos <= 8){
                fil_aux=-2;
                col_aux=0;
                for(int i=4 ; i<=6 ; i++){
                    mapaResultado[fila+fil_aux][columna+col_aux] = sensores.terreno[i];
                    col_aux++;
					pos++;
                }
                mapaResultado[fila-1][columna+2] = sensores.terreno[7];
                mapaResultado[fila][columna+2] = sensores.terreno[8];
            }
            if(pos >=9){
                fil_aux=-3;
                col_aux=0;
                for(int i=9;i<=12 ; i++){
                    mapaResultado[fila+fil_aux][columna+col_aux] = sensores.terreno[i];
                    col_aux++;
					pos++;
                }
                mapaResultado[fila-2][columna+3] = sensores.terreno[13];
                mapaResultado[fila-1][columna+3] = sensores.terreno[14];
                mapaResultado[fila][columna+3] = sensores.terreno[15];
            }
        }else if( brujula == 3 ){
            if( pos >= 1 and pos <= 3){
                col_aux = 1;
                fil_aux = 0;
                for(int i=1;i<=2;i++){
                    mapaResultado[fila+fil_aux][columna+col_aux] = sensores.terreno[i]; 
                    fil_aux++;
					pos++;
                }
                mapaResultado[fila+1][columna] = sensores.terreno[3];
            }
            if( pos >= 4 and pos <= 8){
                fil_aux=0;
                col_aux=2;
                for(int i=4 ; i<=6 ; i++){
                    mapaResultado[fila+fil_aux][columna+col_aux] = sensores.terreno[i];
                    fil_aux++;
					pos++;
                }
                mapaResultado[fila+2][columna+1] = sensores.terreno[7];
                mapaResultado[fila+2][columna] = sensores.terreno[8];
            }
            if(pos >=9){
                fil_aux=0;
                col_aux=3;
                for(int i=9;i<=12 ; i++){
                    mapaResultado[fila+fil_aux][columna+col_aux] = sensores.terreno[i];
                    fil_aux++;
					pos++;
                }
                mapaResultado[fila+3][columna+2] = sensores.terreno[13];
                mapaResultado[fila+3][columna+1] = sensores.terreno[14];
                mapaResultado[fila+3][columna] = sensores.terreno[15];
            }
        }else if( brujula == 5 ){
            if( pos >= 1 and pos <= 3){
                col_aux = 0;
                fil_aux = 1;
                for(int i=1;i<=2;i++){
                    mapaResultado[fila+fil_aux][columna+col_aux] = sensores.terreno[i]; 
                    col_aux--;
					pos++;
                }
                mapaResultado[fila][columna-1] = sensores.terreno[3];
            }
            if( pos >= 4 and pos <= 8){
                fil_aux=2;
                col_aux=0;
                for(int i=4 ; i<=6 ; i++){
                    mapaResultado[fila+fil_aux][columna+col_aux] = sensores.terreno[i];
                    col_aux--;
                }
                mapaResultado[fila+1][columna-2] = sensores.terreno[7];
                mapaResultado[fila][columna-2] = sensores.terreno[8];
            }
            if(pos >=9){
                fil_aux=3;
                col_aux=0;
                for(int i=9;i<=12 ; i++){
                    mapaResultado[fila+fil_aux][columna+col_aux] = sensores.terreno[i];
                    col_aux--;
					pos++;
                }
                mapaResultado[fila+2][columna-3] = sensores.terreno[13];
                mapaResultado[fila+1][columna-3] = sensores.terreno[14];
                mapaResultado[fila][columna-3] = sensores.terreno[15];
            }
        }else if( brujula == 7){
            if( pos >= 1 and pos <= 3){
                col_aux = -1;
                fil_aux = 0;
                for(int i=1;i<=2;i++){
                    mapaResultado[fila+fil_aux][columna+col_aux] = sensores.terreno[i]; 
                    fil_aux--;
					pos++;
                }
                mapaResultado[fila-1][columna] = sensores.terreno[3];
            }
            if( pos >= 4 and pos <= 8){
                fil_aux=0;
                col_aux=-2;
                for(int i=4 ; i<=6 ; i++){
                    mapaResultado[fila+fil_aux][columna+col_aux] = sensores.terreno[i];
                    fil_aux--;
					pos++;
                }
                mapaResultado[fila-2][columna-1] = sensores.terreno[7];
                mapaResultado[fila-2][columna] = sensores.terreno[8];
            }
            if(pos >=9){
                fil_aux=0;
                col_aux=-3;
                for(int i=9;i<=12 ; i++){
                    mapaResultado[fila+fil_aux][columna+col_aux] = sensores.terreno[i];
                    fil_aux--;
					pos++;
                }
                mapaResultado[fila-3][columna-2] = sensores.terreno[13];
                mapaResultado[fila-3][columna-1] = sensores.terreno[14];
                mapaResultado[fila-3][columna] = sensores.terreno[15];
            }
        }
        
    }
    
    
}
void ComportamientoJugador::CompruebaPlan(Sensores sensores,list<Action> &plan){
	if(sensores.terreno[2] == 'A' and !bikini){
		estado objetivo;
		buscaCasilla(objetivo);
		pathFinding_costeUniforme(actual,objetivo,plan,sensores);
	}
	if(sensores.terreno[2] == 'B' and !zapatillas){
		estado objetivo;
		buscaCasilla(objetivo);
		pathFinding_costeUniforme(actual,objetivo,plan,sensores);
	}
	if(sensores.terreno[2] == 'M'){
		estado objetivo;
		buscaCasilla(objetivo);
		pathFinding_costeUniforme(actual,objetivo,plan,sensores);
	}
	if(sensores.terreno[2] == 'P'){
		estado objetivo;
		buscaCasilla(objetivo);
		pathFinding_costeUniforme(actual,objetivo,plan,sensores);
	}
	if(sensores.terreno[2] == 'D'){
		zapatillas = true;
		bikini = false;
	}
	if(sensores.terreno[2] == 'K'){
		bikini = true;
		zapatillas = false;
	}
	
}
void ComportamientoJugador::buscaCasilla(estado &destino){
	destino.columna=-1;
	bool encontrado = false;
	for(int i=0; i< tamanio_mapa and !encontrado ; i++){
		for(int j=0;j<tamanio_mapa and !encontrado ; j++){
			if(mapaResultado[i][j] == '?') {
				destino.fila = i;
				destino.columna = j;	
				encontrado = true;
			}
		}
	}
	
}
void ComportamientoJugador::InicioJuego(Sensores sensores){

    if(inicio_juego){
        for(int i=0;i<tamanio_mapa;i++){
            for(int j=0;j<3;j++){
                mapaResultado[i][j] = 'P';
            }
            for(int j=tamanio_mapa-3;j<tamanio_mapa;j++){
                mapaResultado[i][j] = 'P';
            }
        }
        for(int j=0;j<tamanio_mapa;j++){
            for(int i = 0; i<3;i++){
                mapaResultado[i][j] = 'P';
            }            
            for(int i = tamanio_mapa-3;i<tamanio_mapa;i++){
                mapaResultado[i][j] = 'P';
            }
        }
        estado obje2;
        buscaCasilla(obje2);
        pathFinding_costeUniforme(actual,obje2,plan,sensores);
        inicio_juego=false;
    }

}
// Metodos nivel 4, maximos objetivos
void ComportamientoJugador::ActualizaBrujula(int &fil,int &col, int ori, Action &accion){

	// calculo cual es la casilla de delante del agente
	switch (accion){
		case actFORWARD:
			switch (orientacion) {
				case 0:
					fil--;
					break;
				case 1:
					fil--;
					col++;
					break;
				case 2:
					col++;
					break;
				case 3:
					fil++;
					col++;
					break;
				case 4:
					fil++;
					break;
				case 5:
					fil++;
					col--;
					break;
				case 6:
					col--;
					break;
				case 7:
					fil--;
					col--;
					break;
				}
			break;
		case actTURN_L:
			orientacion = (orientacion + 6) % 8;
			break;
		case actTURN_R:
			orientacion = (orientacion + 2) % 8;
			break;
		case actSEMITURN_L:
			orientacion = (orientacion + 7) %8;
			break;
		case actSEMITURN_R:
			orientacion = (orientacion + 1) %8;
			break;
	}	
}