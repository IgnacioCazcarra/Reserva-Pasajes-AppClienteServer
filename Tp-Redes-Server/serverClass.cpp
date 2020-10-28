#include "serverClass.h"
#include "leerArchivo.h"
#include "escribirArchivo.h"
#include <iostream>
#include <winsock2.h>
#include <string>
#include <conio.h>
#include <clocale>//es para usar � y acento
#include <fstream> //Lib. para trabajar con archivos
#include <ctime> //Lib. para trabajar con fechas / tiempos
#include <cstdlib>
#include <vector>
#include <stdio.h>
#include <windns.h>

#define TAMANIO_I  5
#define TAMANIO_J  21

using namespace std;

vector<string> archivos_servicios;


/**********************************************************************/
void renovacionDeMicrosDisponibles(){
    vector <string> vecStringAux;
    if(verificarSiExisteArchivo("Archivos_activos")){
            string servicio = "";
            ifstream archivoServiciosRespaldo;
            archivoServiciosRespaldo.open("Archivos_activos.txt" ,ios::in);
            while(!archivoServiciosRespaldo.eof()){ ///el archivo  puede contener hasta 6 lineas , cada linea hace refencia a un servicio
                getline(archivoServiciosRespaldo , servicio);
                if(servicio != ""){
                   if(siFechaActualEsMayor(servicio)==false){//si los servicios tienen fechas que son igual o mayor a la fecha actual
                      archivos_servicios.push_back(servicio);//guardo el  servicio en el vector GLOBAL
                      vecStringAux.push_back(servicio);//guardo el  servicio en el vector que uso en el if (vecStringAux.size()>0)
                   }else{//si la fecha actual es mayor
                     registrarViajesEnArchivo(servicio);//los paso a un archivo general (servicio realizado)
                     servicio=servicio+".txt";
                     remove(servicio.c_str());//borro el archivo individual
                   }
                }

                //limpiamos la cadena para la prox iteracion
                servicio.clear();
            }
            archivoServiciosRespaldo.close();

             if(vecStringAux.size()>0){//si qued� para dejar en el archivo "Archivos_activos"
                   actualizarCambiosEnArchivo(vecStringAux,"Archivos_activos");//saco lo que pas� al archivo definitivo
            }else if(vecStringAux.size()==0){ remove("Archivos_activos.txt"); }//Si no qued� ni un registro con fecha igual o superior a la actual BORRO EL ARCHIVO y se generar� cuado sea necesario

    }else{
       cout << "Error, no existe el archivo: Archivos_activos.txt"<< endl;
       exit(1);
    }
}
/************************************************************************/


/***********************************************************************/
void crearServicio(string userName , Server*& servidor){
    /***Datos recibidos del  usuario****/
    string fechaRecibida = servidor->Recibir();
    string origenRecibido = servidor->Recibir();
    string turnoRecibido  = servidor->Recibir();
    /*****************************/
    string msg = "";
    string nombreArchivo = "";
    string tituloArchivo = "";
    if(origenRecibido=="Mar del Plata"){nombreArchivo=fechaRecibida+";Mar_Del_Plata;"+turnoRecibido; tituloArchivo=fechaRecibida+" "+"Mar_Del_Plata"+" "+turnoRecibido;}
    else if(origenRecibido=="Buenos Aires"){nombreArchivo=fechaRecibida+";Buenos_Aires;"+turnoRecibido; tituloArchivo=fechaRecibida+" "+"Buenos_Aires"+" "+turnoRecibido;}

    if(crearArchivoButacas(nombreArchivo , tituloArchivo)){ ///si el servicio no existe crea su registro con sus correspondientes datos
        registrarUserLog("Crea el servicio con los datos ("+tituloArchivo+")" , userName);///registro la accion del usuario en su archivo
        guardarEnArchivoSinFormato(nombreArchivo, "Archivos_activos"); ///registro el servicio creado en un archivo de respaldo

        msg = "El Servicio ("+tituloArchivo+") fue creado correctamente";
        archivos_servicios.push_back(nombreArchivo); ///agrego el nombre del archivo en el vector
    }
    else{
            msg = "El Servicio ("+tituloArchivo+") que quiere crear ya existe";
    }

        servidor->Enviar(msg); ///le informa al usuario el resultado de la operacion
}
/***********************************************************************/


void registrarServerLog(string evento, string aRegistrar){
    std::ofstream serverLog("server.log", std::ios::ate | std::ios::in);
    if(serverLog.fail()){ //Si el archivo no se encuentra o no esta disponible o presenta errores
            cout<<"No se pudo abrir el archivo server log"; //Muestra el error
                        }
    time_t     now = time(0);
    struct tm  tstruct;
    char       buf[80];
    tstruct = *localtime(&now);
    strftime(buf, sizeof(buf), "%Y-%m-%d.%X", &tstruct);
    serverLog<<buf;
    serverLog<<": "<<evento<<" - "<<aRegistrar<<endl;
    serverLog<<": ==================================="<<endl;
    serverLog.close();
}


void verificarArchivoServerLog(){
    string nombreArchivo = "server.log";
  std::ifstream serverLog( nombreArchivo );
  if(serverLog.fail()){
    //EL ARCHIVO NO EXISTE
    std::ofstream serverLogCrear( nombreArchivo );
  }

  serverLog.close();
}
/***********************************************************************/


/***********************************************************************/
void mostrarRegistro(string userName, Server *&Servidor){

    std::string userFile = userName+".log";
    std::string numero = std::to_string(numeroDeSentencias(userFile));
    Servidor->Enviar(numero);
    fstream file;
    file.open(userFile);

    if(file.is_open()){
        for(int i = 0 ; i < stoi(numero); i++){
            string linea = "";
            getline(file, linea);
            Servidor->Enviar(linea);
        }
    }
    file.close();

}
/***********************************************************************/


string checkUser(Server *&Servidor)
{
    string usuarioEncontrado = "false";
    vector<string> resultados;
    vector<string> userAndPass;
    int contador = 0;
    string loggedUser = "";


    while(contador<3 && usuarioEncontrado == "false"){

        string linea;
        fstream file;

        file.open("users.dat", ios::in);

        userAndPass = separarPalabrasPuntoYComa(Servidor->Recibir());

        if(file.is_open())
        {
            while(!file.eof()){

                getline(file, linea);

                resultados = separarPalabrasPuntoYComa(linea);

                if(resultados[0] == userAndPass[0] && resultados[1] == userAndPass[1]){
                        usuarioEncontrado = "true";
                        verificarArchivoServerLog();
                        registrarServerLog("Usuario autenticado", resultados[0]);
                        crearArchivoUserLog(resultados[0]);
                        registrarUserLog("Inicia sesion", resultados[0]);
                        loggedUser = resultados[0];
                }


            }
        }

        file.close();

        usuarioEncontrado == "true" ? cout<<"Usuario Encontrado"<<endl<<endl : cout<<"Crendenciales invalidas..."<<endl<<endl<<"Por favor ingrese sus datos nuevamente (Le quedan " << " intentos)"<<endl<<endl;

        contador++;

        if(usuarioEncontrado == "true") {
            contador = 4;
        }

        Servidor->Enviar(usuarioEncontrado+";"+to_string(contador));

    }
  return loggedUser;
}


/***********************************************************************/
void gestionarAsiento(string nombreArchivo,Server *&Servidor, string userName, bool reservar){

    vector <string> vectorButacas = leerArchivoGuardarEnVectorString(nombreArchivo);

    Servidor->Enviar(traerSoloButacas(vectorButacas));

    string salir = "false";

    bool salirWhile = false;

    while(!salirWhile){

        vectorButacas = leerArchivoGuardarEnVectorString(nombreArchivo);
        salir = verificarSolicitud_Y_Responder(Servidor,vectorButacas, userName, reservar);

        if(salir=="true"){
            salirWhile=true;
        }
        else if(salir!="true" && salir!="false"){//es porque cuando se desconecta el cliente  cerrando la ventana llegaba vacio
            salir ="true";
            salirWhile = true;
            system("cls");
        }
    }

}
/***********************************************************************/
void liberar(Server *&Servidor, string userName){

}

/***********************************************************************/


/***********************************************************************/
string verificarSolicitud_Y_Responder(Server *&Servidor,vector <string> vectorButacas, string userName, bool reservar){
    string mensajePeticion = "";
    string mensajeDelCli="";
    char letra = '\0';
    int pos_J = -1;
    int pos_I = -1;

    mensajeDelCli = Servidor->Recibir(); //Se recibe la butaca. EJ: B8

    if(mensajeDelCli!="0"){
        letra = mensajeDelCli[0];
        mensajeDelCli.erase(0,1);//Saco la letra que guerd�
        pos_J =atoi(const_cast< char *>(mensajeDelCli.c_str()));
        pos_J = pos_J*2+2; //es por la diferencia que hay entre la posicion de vista en consola y la del archivo
        pos_I = asignarValorPosI_A_Letra(letra);


        if(vectorButacas[pos_I][pos_J]=='O' && reservar){
           Servidor->Enviar("Disponible");//est� disponible

           marcarButacaComoOcupada(vectorButacas, pos_I, pos_J, userName);
           mensajePeticion = Servidor->Recibir();
        }
        else if(vectorButacas[pos_I][pos_J]=='X' && !reservar){
            Servidor->Enviar("Disponible");//est� disponible

            marcarButacaComoLiberada(vectorButacas, pos_I, pos_J, userName);
            mensajePeticion = Servidor->Recibir();
        }
        else{
            Servidor->Enviar("NoDisponible");//est� disponible
            //mostrarButacas(vectorButacas);
            mensajePeticion = Servidor->Recibir();
        }
    }
    else{
        mensajePeticion = Servidor->Recibir();
    }

    return mensajePeticion;
}
/***********************************************************************/

/***********************************************************************/
void manejarPeticion(string nombreArchivo,string userName, Server *&Servidor){
    string peticion="";
    bool salir = false;
    string opcionesPosibles=" ";
    while(!salir){
      salir = true;
        peticion = Servidor->Recibir();
 //       cout<<peticion<<endl;
        //Servidor->Enviar("_");//Envio cualquier cosa para que no d� error

        if(peticion=="Registro"){
            mostrarRegistro(userName,Servidor);
            salir = false;
        }
        else if(peticion=="AltaServicio"){
            crearServicio(userName, Servidor);
            salir = false;
        }
        else if(peticion=="Gestionar"){
            salir = false;
            string opcion = Servidor->Recibir();
            if(opcion=="ReservarAsiento"){
                gestionarAsiento(nombreArchivo,Servidor, userName,true);
                opcion = "";     salir = false;
            }
            else if(opcion=="LiberarAsiento"){
                gestionarAsiento(nombreArchivo,Servidor, userName, false);
                opcion = "";     salir = false;
            }
        }
        peticion="";


    }
}
/***********************************************************************/


/***********************************************************************/
int numeroDeSentencias(std::string archivo){

        int contador = 0;

        fstream file;

        file.open(archivo);

        if(file.is_open()){
            while(!file.eof()){
                string linea = "";
                getline(file, linea);
                contador++;
            }
        }
        file.close();
    return contador;
};
/***********************************************************************/