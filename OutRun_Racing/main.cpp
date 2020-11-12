// pour cette analyse et les suivantes, je vais aller plus vite et donc je ne comprends et v�rifie pas tout.
// le but reste d'identifier la logique globale et les fonctions SFML.
// v�rifier chaque equation de mouvement ou de collision serait trop chronophage


// on voit du blanc en bas et � droite. 
// quand la voiture d�passe en haut et � gauche, il semble y avoir un bug

#include <SFML/Graphics.hpp>
using namespace sf;


// les points sur la carte par lesquels devront passer les voitures IA

const int num=8; //checkpoints
int points[num][2] = {300, 610,
                      1270,430,
                      1380,2380,
                      1900,2460,
                      1970,1700,
                      2550,1680,
                      2560,3150,
                      500, 3300};


// c'est ce qui me semblait: une struct en c++ est une classe o� tout est public: il n'y a pas seulement attribut mais aussi
// des m�thodes possibles comme ici les m�thodes move et find target

struct Car
{
  float x,y,speed,angle; int n;

  Car() {speed=2; angle=0; n=0;}


  // gestion trigonometrique du mouvement, je fais confiance.
  void move()
   {
    x += sin(angle) * speed;
    y -= cos(angle) * speed;
   }


  // cette fonction permet aux voitures IA de se deplacer vers les cibles les unes apr�s les autres (n)
  // il y a une correction d'angle avec de la trigonometrie pour g�rer les trajectoires, je ne vais pas rentrer la dedans

  void findTarget()
  {
    float tx=points[n][0];
    float ty=points[n][1];
    float beta = angle-atan2(tx-x,-ty+y);
    if (sin(beta)<0) angle+=0.005*speed; else angle-=0.005*speed;
    if ((x-tx)*(x-tx)+(y-ty)*(y-ty)<25*25) n=(n+1)%num;
   }
};


int main()
{
    RenderWindow app(VideoMode(640, 480), "Car Racing Game!");
    app.setFramerateLimit(60);

    Texture t1,t2,t3;
    t1.loadFromFile("images/background.png");
    t2.loadFromFile("images/car.png");
    t1.setSmooth(true);
    t2.setSmooth(true);


    // il est int�ressant de remarquer ce que j'avais vu avant que le sprite evidemment n'est pas l'objet ni la texture.
    Sprite sBackground(t1), sCar(t2);

    // param�tre rigolo reprensentant un zoom du background. jouer � 4,4 est presque plus simple!
    sBackground.scale(2,2);

    // point important: on defini le centre de la voiture (22,22) (sur le sprite!!) et non pas l'angle � 0,0 . Pour les calculs
    // ca aura certainement son importance.

    sCar.setOrigin(22, 22);

    // peut-�tre le "rayon?"
    float R=22;


    // il y a 5 voitures en compet
    const int N=5;
    Car car[N];
    for(int i=0;i<N;i++)
    {
      car[i].x=300+i*50; // position diff�rente gr�ce au i
      car[i].y=1700+i*80;
      car[i].speed=7+i; // vitesse diff�rente en utilisant le i
    }

// param�tres des mouvements de la voiture

   float speed=0,angle=0;
   float maxSpeed=12.0;
   float acc=0.2, dec=0.3;
   float turnSpeed=0.08;

   // montre la partie du background pour que la voiture reste globablement au centre
   int offsetX=0,offsetY=0;

    while (app.isOpen())
    {
        Event e;
        while (app.pollEvent(e))
        {
            if (e.type == Event::Closed)
                app.close();
        }
// une m�thode bizarre de gestion des evenement clavier (fleche)
// quatre booleen initilias� � chaque it�ration � zero
// puis met "1" dans le booleen correspondant � la fl�che (bas droite, gauche, haut).
        
    bool Up=0,Right=0,Down=0,Left=0;
    if (Keyboard::isKeyPressed(Keyboard::Up)) Up=1;
    if (Keyboard::isKeyPressed(Keyboard::Right)) Right=1;
    if (Keyboard::isKeyPressed(Keyboard::Down)) Down=1;
    if (Keyboard::isKeyPressed(Keyboard::Left)) Left=1;

    //car movement

    // si on appuie sur UP, la vitesse augmente progressivement jusqu'� vitesse max 
    // en incr�mentant speed avec acc.

    if (Up && speed<maxSpeed)
        if (speed < 0)  speed += dec; // depend du sens dans lequel avance la voiture (monte=diminue y; descend = aug�ente y)
        else  speed += acc;


    // si on appuie sur Down, c'est le comportement inverse de up (marche arri�re)

    if (Down && speed>-maxSpeed)
        if (speed > 0) speed -= dec; 
        else  speed -= acc; 

// J'imagine que la voiture s'arr�te progressivement

    if (!Up && !Down)
        if (speed - dec > 0) speed -= dec; // deceleration progressive dans un sens?
        else if (speed + dec < 0) speed += dec; // deceleration progressive dans l'autre sens?
        else speed = 0;

// si la voiture "tourne" (fleche gauche et droite) , on accroit ou decroit l'angle selon gauche ou droite.
// l'equation a probablment �t� choisi pour illustrer la r�alit�:  l'angle semble �tre d'autant plus grand
// qu'on va vite (contre intuitif)?).

    if (Right && speed!=0)  angle += turnSpeed * speed/maxSpeed;
    if (Left && speed!=0)   angle -= turnSpeed * speed/maxSpeed;


// une fois que les param�tres speed et angle ont �t� ajust�, on les ajuste dans notre objet car
// c'est � dire la car[0] qui est la notre. Les autres voitures ont probablement une vitesse constante
// seul leur angle change.

    car[0].speed = speed;
    car[0].angle = angle;


// Puis on appelle la fonction move qui va modifier x et y de la voiture en fonction de la vitesse et de l'angle 
// c'est dans cette fonction move qu'on trouve sin et cos
// toutes les voitures bougent...

    for(int i=0;i<N;i++) car[i].move();

// mais seuls les voitures IA cherchent leur cible (la boucle for commence � 1 et pas � zero).

    for(int i=1;i<N;i++) car[i].findTarget();

//collision
    // probable redondance des deux boucles for? voiture 1 touche voiture 5 et voiture 5 touche voiture 1?)
    for(int i=0;i<N;i++)
    for(int j=0;j<N;j++)
    {      
        int dx=0, dy=0;

        // si la somme des diff�rence (voir plus bas) est trop petite, ca veut dire que les voitures se touchent
        while (dx*dx+dy*dy<4*R*R) // doit plus ou moins correspondre � une aire o� les voitures se touchent/superposent).
         {
           car[i].x+=dx/10.0;// il faut donc les faire se d�caler progressivement en x et y ; voiture i
           car[i].x+=dy/10.0;
           car[j].x-=dx/10.0;// mais dans un sens oppos� ! (le - ); voiture j
           car[j].y-=dy/10.0;
           dx = car[i].x-car[j].x;// c'est la diff�rence entre les deux voiture (en x)
           dy = car[i].y-car[j].y;// c'est la diff�rence entre les deux voiture (en y)
           if (!dx && !dy) break; // ?
         }
    }


    app.clear(Color::White);

    // la taille du background fait 640* 480. 
    // quand la voiture d�passer la moiti� de l'�cran en x ou y
    // on change l'offset relativement � la position de la voiture
    // ce qui fait que la majorit� du temps, la voiture reste au centre et le background bouge derri�re
    // sauf sur les bords. bizarrement, j'ai du blanc que sur la face droite.
    // pas encore bien clair

    if (car[0].x>320) offsetX = car[0].x-320;
    if (car[0].y>240) offsetY = car[0].y-240;

    // le background fait 1440 * 1840 soit bien plus que le fen�tre 640*480 et en plus on zoom X2 pour l'echelle du background
    // Tout le background est affich�, je pense mais seul la partie dans la fen�tre est visible (pas s�r)/

    // on modifie donc en temps r�el la position du background mais j'avoue que je pige pas bien le -offset

    sBackground.setPosition(-offsetX,-offsetY);
    app.draw(sBackground);

    Color colors[10] = {Color::Red, Color::Green, Color::Magenta, Color::Blue, Color::White};

    for(int i=0;i<N;i++)
    {
      sCar.setPosition(car[i].x-offsetX,car[i].y-offsetY); // corrige pour que toujours au centre? un truc du genre.
      sCar.setRotation(car[i].angle*180/3.141593);
      sCar.setColor(colors[i]); // il semble qu'on puisse rajouter une couleur sur une texture d'image.
      app.draw(sCar);
    }

    app.display();
    }

    return 0;
}
