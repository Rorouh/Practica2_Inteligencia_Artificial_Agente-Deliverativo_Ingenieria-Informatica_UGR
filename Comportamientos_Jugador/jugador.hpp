#ifndef COMPORTAMIENTOJUGADOR_H
#define COMPORTAMIENTOJUGADOR_H

#include "comportamientos/comportamiento.hpp"

#include <list>
#include <queue>

struct estado {
  int fila;
  int columna;
  int orientacion;
  bool bikini_aux;
  bool zapatillas_aux;
};

class ComportamientoJugador : public Comportamiento {
  public:
    ComportamientoJugador(unsigned int size) : Comportamiento(size) {
      // Inicializar Variables de Estado
      hayPlan=false;
      //Nivel 3
      zapatillas = false;
      bikini =false;
      tamanio_mapa = size;
      inicio_juego=true;
      fila = 0;
      columna = 0;
      orientacion = 0;
      principio = true;
      //
    }
    ComportamientoJugador(std::vector< std::vector< unsigned char> > mapaR) : Comportamiento(mapaR) {
      // Inicializar Variables de Estado
      hayPlan=false;
      zapatillas = false;
      bikini =false;
    }
    ComportamientoJugador(const ComportamientoJugador & comport) : Comportamiento(comport){}
    ~ComportamientoJugador(){}

    Action think(Sensores sensores);
    int interact(Action accion, int valor);
    void VisualizaPlan(const estado &st, const list<Action> &plan);
    ComportamientoJugador * clone(){return new ComportamientoJugador(*this);}

  private:
    // Declarar Variables de Estado
    estado actual;
    list<estado> objetivos;
    list<Action> plan;
    //nivel 3 o 4
    
    bool bikini;
    bool zapatillas;
    int tamanio_mapa;
    bool inicio_juego;
    int fila,columna,orientacion;
    bool principio;
    //
    bool hayPlan;

    // Métodos privados de la clase
    bool pathFinding(int level, const estado &origen, const list<estado> &destino, list<Action> &plan,Sensores sensores);
    bool pathFinding_Profundidad(const estado &origen, const estado &destino, list<Action> &plan);
    bool pathFinding_anchura(const estado &origen, const estado &destino, list<Action> &plan);
    bool pathFinding_costeUniforme(const estado &origen, const estado &destino, list<Action> &plan, Sensores sensores);
    void Vision(Sensores sensores);
    void GuardaUltimaAccion(Action accion,Sensores sensores, bool situado,int fil_aux,int col_aux);
    bool descubre_mapa(Sensores sensores,list<Action> &plan);
    int CalculaCoste(estado estado_aux, Action accion);
    int CalculaZona(Sensores Sensores);
    void InicioJuego(Sensores sensores);
    void CompruebaCasilla(Sensores sensores);
    void CompruebaPlan(Sensores sensores,list<Action> &plan);
    void buscaCasilla(estado &destino);
    void CalculaObjetivo(estado &objetivo_aux, Sensores sensores);
    void ActualizaBrujula(int &fil,int &col, int ori, Action &accion);

    void PintaPlan(list<Action> plan);
    bool HayObstaculoDelante(estado &st);


};

#endif
