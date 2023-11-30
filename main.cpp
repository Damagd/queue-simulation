#include <stdio.h>
#include <stdlib.h>
#include <graphics.h>
#include <string>
#include <time.h>
#include <iostream>

typedef struct
{
    int T1;
    int T2;
    int TT=0;
    int TG=0;
}TTiempo;

typedef struct
{
    void *imagen;
    int imagentam;
}TSprite;

typedef struct nodo
{
    int x;
    int xp;
    int y;
    int ya;
    int nombre;
    int color;
    int tiempo=0;
    int esperar=0;
    nodo *sig;
    nodo *ant;
}TNodo;//Persona

typedef struct
{
    int x=128;
    int tiempo=0;
    int disponible=1;
    TNodo *ant;
    int nombre;
    int color;
}TCaja;

typedef struct
{
    TNodo *ini;
    TNodo *fin;
}TCola;

void iniciaCola(TCola *c);
void guardaSprite(TSprite *spr,char nom[15]);
int insertaInicioLisD(TTiempo tG,TCola *c);
TNodo *creaNodoD();
void dibuja(TCola cola,TSprite spr[],TCaja cajas[],TSprite sprc[]);
void ControlLlegada(TTiempo *tG,TCola *c);
void actualizaEstado(TTiempo tG,TCola *c,TCaja cajas[]);
int eliminaFinal(TCola *c);
void ControlSalida(TCola *c,TCaja cajas[]);
void LLevaCajas(TCaja caja,TNodo *c);
void ControlIrse(TCola *cola,TCaja cajas[]);
void Pausa();
int eliminaReferencia(TCola *C);
void cargaSprite(TSprite spr[],TSprite sprc[]);

int main()
{
    TCola cola;
    TCaja cajas[5];
    TTiempo tiempoG;
    TSprite sprite[3];
    TSprite spritec[2];
    int v=0;
    //Pantalla
    initwindow(576,576);
    setbkcolor(RGB(240,240,240));
    setfillstyle(10,15);

    int del=200,a,i;

    iniciaCola(&cola);

    cargaSprite(sprite,spritec);
    setcolor(BLACK);
    setactivepage(1);
    while(true)
    {
        setactivepage(v);
        cleardevice();

        Pausa();

        tiempoG.T1=time(0);
        ControlLlegada(&tiempoG,&cola);
        ControlSalida(&cola,cajas);
        ControlIrse(&cola,cajas);
        dibuja(cola,sprite,cajas,spritec);
        delay(del);

        tiempoG.T2=time(0);
        tiempoG.TT=tiempoG.T2-tiempoG.T1;
        tiempoG.TG+=tiempoG.TT;

        actualizaEstado(tiempoG,&cola,cajas);
        setvisualpage(v);
        v=!v;
    }


    getch();

}

void iniciaCola(TCola *c)
{
    c->ini=NULL;
    c->fin=NULL;
}

/*Funci�n que llama y controla lo que se va a pasar a guardaSprite*/
void cargaSprite(TSprite spr[],TSprite sprc[])
{
    int i,j=0;
    char nombres[5][15]={"playerF.txt","playerB.txt","playerL.txt","cajaNoD.txt","cajaDisp.txt"};
    for(j=i=0;i<3;i++,j++) guardaSprite(&spr[i],nombres[j]);
    for(i=0;i<2;i++,j++) guardaSprite(&sprc[i],nombres[j]);
}

/*Funci�n para dibujar el archivo que se paso a la pantalla, y despues guardarlo*/
void guardaSprite(TSprite *spr,char nom[15])
{
    FILE *a;
    int i,j,e;
    a=fopen(nom,"r");

    fscanf(a,"%d",&e);
    fscanf(a,"%d",&e);

    for(i=0;i<32;i++)
    {
        for(j=0;j<32;j++)
        {
            fscanf(a,"%d",&e);
            putpixel(j,i,e);
        }
    }

    fclose(a);
    spr->imagentam=imagesize(0,0,31,31);
    spr->imagen=malloc(spr->imagentam);
    getimage(0,0,31,31,spr->imagen);
}

/*Funci�n que checa si se presion� la barra espaciadora, s� se presiono, entonces detiene el programa hasta que se presione de nuevo.*/
void Pausa()
{
    int a=0;
    int i=0;
    if(kbhit())
    {
        a=getch();
        if(a==224)getch();
        if(a==32)
            i=1;
        a=0;
        while(i==1)
        {
            setactivepage(2);
            setvisualpage(2);
            setbkcolor(BLACK);

            if(kbhit())a=getch();
            if(a==32)i=0;
            setcolor(WHITE);
            outtextxy(getmaxx()/2,getmaxy()/2,"PAUSA");

        }
        setactivepage(0);
        setvisualpage(0);
        setbkcolor(RGB(240,240,240));
    }
}

/*Funci�n para controlar si se agrega un nuevo nodo o no.*/
void ControlLlegada(TTiempo *tG,TCola *c)
{
    int a;
    srand(time(0));
    a=rand()%5;

    //S� la cola esta vac�a, entonces que siempre agregue un nodo desde el inicio
    if(!c->ini)
    {
        insertaInicioLisD(*tG,c);
    }

    //S� ya pasaron 5 segundos o mas y el numero aleatorio es 0 y ademas, no es el fin de pantalla, entonces agregar otro nodo
    if((tG->TG>=5 && a==0) && c->ini->y+32<=576)
    {
        insertaInicioLisD(*tG,c);
        tG->TG=0;
    }
}

/*Funci�n para introducir a las personas que van llegando a la fila*/
int insertaInicioLisD(TTiempo tG,TCola *c)
{
    int ret=0;
    TNodo *nuevo;

    nuevo=creaNodoD();
    if(nuevo)
    {
        if(c->ini==NULL)
        {
            c->ini=c->fin=nuevo;
            nuevo->xp=nuevo->x=getmaxx()/2;
            nuevo->y=getmaxy()/2;
        }else
        {
            nuevo->xp=nuevo->x=c->ini->x=getmaxx()/2;
            nuevo->y=c->ini->y+32;
            nuevo->sig=c->ini;
            c->ini->ant=nuevo;
            c->ini=nuevo;
        }
        ret++;
    }
    return ret;
}

/*Funci�n para controlar si una "persona" va a pasar a la "caja".*/
void ControlSalida(TCola *c,TCaja cajas[])
{
    int i;

    //Recorrido para detectar que caja esta disponible.
    for(i=0;i<5 && cajas[i].disponible!=1;i++);

    //S� hay cajas (i<5) y el nodo o persona que se encuentra al principio de la fila aun quiere esperar su turno (c->fin->esperar==1) y ya camino hasta el frente (c->fin->ya+31==c->fin->y)
    if(i<5 && (c->fin->esperar==1 && c->fin->ya+31==c->fin->y))
    {
        //Entonces pasa el nombre y color de la persona a la caja, dibuja una linea para observar a cual fue, elimina a la persona o nodo, y deshabilita la caja
        cajas[i].nombre=c->fin->nombre;
        cajas[i].color=c->fin->color;
        setcolor(c->fin->color);
        setlinestyle(1,1,3);
        line(c->fin->x,c->fin->ya,(115*i)+16,32);
        eliminaFinal(c);
        cajas[i].disponible=0;
        cajas[i].tiempo=0;
    }
}

/*Funci�n para eliminar o sacar a una persona que ya pas� a caja*/
int eliminaFinal(TCola *c)
{
    TNodo *aux;
    int band=0;

    if(c->fin)
    {
        if(c->ini==c->fin)
        {
            free(c->ini);
            c->ini=c->fin=NULL;
        }else
        {
            aux=c->fin->ant;
            aux->y=c->fin->y;

            aux->sig=NULL;
            free(c->fin);
            c->fin=aux;
        }
        band++;
    }
    return(band);
}

/*Funci�n para controlar que persona decidio no esperar*/
void ControlIrse(TCola *cola,TCaja cajas[])
{
    TNodo *aux,*aux2;
    //Ciclo para buscar que persona decidio salirse de la fila
    for(aux=cola->fin;aux && aux->esperar==1;aux=aux->ant);
    if(aux) //S� lo encontro
    {
        //Trozo de codigo para hacer que las personas se recorran desde el ultimo que entro a la fila hasta el que se tiene que salir, para que la persona que desea salir, pueda hacerlo
        for(aux2=cola->ini;aux2 && aux2->x==aux->x+32 && aux2!=aux;aux2=aux2->sig)
            aux2->esperar=1;

        if(aux2 && aux2!=aux && aux2->x!=aux->x+32)//Si las personas atras no se han recorrido, que lo hagan
        {
            aux2->x+=32;
        }
        if(aux->ya>576)//Si la persona que desea salir ya salio de la pantalla, entonces elimina
        {
            eliminaReferencia(cola);
        }
    }else if(!aux)//Si el nodo no existe, o ya sali� de la fila
    {
        for(aux2=cola->fin;aux2 && aux2->x==aux2->xp;aux2=aux2->ant);//Entonces que recorra a todas las personas para volverlas a su posici�n normal, el ultimo que se recorrio, es el primero en volver
        if(aux2 && aux2->x!=aux2->xp)
        {
            aux2->x=aux2->xp;
        }
    }
}

/*Funci�n para eliminar a una persona que sali� de la fila*/
int eliminaReferencia(TCola *C)
{
    int band=0;
    TNodo *aux;

    if(C->ini)
    {
        for(aux=C->fin;aux && aux->esperar!=0;aux=aux->ant);
        if(aux)
        {
            if(aux==C->ini)
            {
                if(aux==C->fin)
                    C->ini=C->fin=NULL;
                else
                {
                    C->ini=aux->sig;
                    aux->sig->ant=NULL;

                }
            }else
            {
                if(C->fin==aux)
                {
                    C->fin=aux->ant;
                    aux->ant->sig=NULL;
                    C->fin->y=aux->y;
                }else
                {
                    aux->ant->sig=aux->sig;
                    aux->sig->ant=aux->ant;
                }
            }
            free(aux);
            band++;
        }
    }
    return band;
}

void actualizaEstado(TTiempo tG,TCola *c,TCaja cajas[])
{
    TNodo *aux;
    int a,f,i;
    srand(time(0));

    //Recorrer a todos los nodos para actualizar sus coordenadas y estados
    for(f=1,aux=c->fin;aux;aux=aux->ant,f++)
    {
        aux->tiempo+=tG.TT;
        a=rand()%20;

        if(aux->sig)
        {
            aux->y=aux->sig->y+32;
        }

        //Condici�n para decidir si seguir esperando o no. Las personas que esten a la mitad de la fila tienen mas posibilidad de irse
        if((aux->tiempo>110 && a==5) || (aux->tiempo>70 && f==5 && a%2==0))
        {
            aux->esperar=0;
        }
    }

    //Ciclo para actualizar el estado de las cajas, s� ya pas� 1 minuto o m�s y el numero aleatorio es 1, entonces se abre
    for(i=0;i<5;i++)
    {
        a=rand()%25;
        cajas[i].tiempo+=tG.TT;
        if(cajas[i].tiempo>=60 && a==1)
        {
            cajas[i].disponible=1;
        }
    }
}

/*Funci�n para dibujar todo lo que se muestra en pantalla*/
void dibuja(TCola cola,TSprite spr[],TCaja cajas[],TSprite sprc[])
{
    TNodo *aux;int i;
    char n[26][30]={"Alan","Britanny","Charlie","Daniela","Esteban","Felipe","Gerardo Ortiz","Hector","Iris","John Wick","Kuck","Light","Mauricio","Natalia","Oscar","Pi�a","Quck","Ricardo","Sofia","Tona","Uriel","Veronica","Woshingo","Xozitl","Yei","Zucko"};

    char out_string [15]={'\n'};

    for(aux=cola.fin;aux;aux=aux->ant)
    {
        sprintf(out_string,"%ds",aux->tiempo);
        if(aux->y<aux->ya && aux->esperar==1)
        {
            aux->ya-=32;
        }else if(aux->esperar==0 && aux->ant && aux->ant->x!=aux->x)
        {
            aux->ya+=32;
        }

        setcolor(aux->color);
        if(aux->esperar==1)
        {
            outtextxy(aux->x+32,aux->ya,n[aux->nombre]);
            outtextxy(aux->x+32,aux->ya+16,out_string);
            if(aux->tiempo>=70)
            {
                outtextxy((aux->x+32)+textwidth(n[aux->nombre]),aux->ya,": Ya tardaron");
            }else
            {
                outtextxy((aux->x+32)+textwidth(n[aux->nombre]),aux->ya,": Espero");
            }
        }else
        {
            outtextxy(aux->x-32,aux->ya,n[aux->nombre]);
            outtextxy((aux->x-32)-textwidth(out_string),aux->ya+16,out_string);
            outtextxy((aux->x-32)-textwidth("Ya me voy -_- :"),aux->ya,"Ya me voy -_- :");
        }

        if(aux->x==aux->xp+32)
            putimage(aux->x,aux->ya,spr[2].imagen,3);
        else putimage(aux->x,aux->ya,spr[aux->esperar].imagen,3);
    }

    for(aux=cola.fin,i=0;i<5;i++)
    {
        sprintf(out_string,"%ds",cajas[i].tiempo);
        cajas[i].x=117*i;
        putimage(cajas[i].x,2,sprc[cajas[i].disponible].imagen,3);
        if(cajas[i].disponible==0)
        {
            setcolor(cajas[i].color);
            putimage(cajas[i].x,34,spr[1].imagen,3);
            outtextxy(cajas[i].x,66,n[cajas[i].nombre]);
            setcolor(RED);
            outtextxy(cajas[i].x+32,16,"No disp");
        }else
        {
            setcolor(GREEN);
            outtextxy(cajas[i].x+32,16,"Disponible");
        }
        outtextxy(cajas[i].x+32,0,out_string);
    }
}

/*Funci�n para crear un nodo*/
TNodo *creaNodoD()
{
    TNodo *nuevo;
    nuevo=(TNodo*)malloc(sizeof(TNodo));
    int e;
    srand(time(0));
    e=rand()%26;
    if(nuevo)
    {
        nuevo->tiempo=0;
        nuevo->nombre=e;
        nuevo->color=rand()%16;
        nuevo->ya=576;
        nuevo->sig=nuevo->ant=NULL;
        nuevo->esperar=1;
    }
    return nuevo;
}
