#if 0

- on a du code dans TMObject du genre

   std::string const* classname = in_typed_object->FindPropertyString("classname");

   ca ressemble a s y meprendre a du JSON. peut etre un pont a faire.

- TMObjectFactory TMLevel::DoGetExplicitObjectFactory(TMLayerInstance* in_layer_instance, TiledMap::TypedObject const * in_typed_object)

  on travail avec "classname" mais on pourrait aussi bien utiliser type non ?

- conversion automatique de float et double

class BiValue
   {

	   BiValue(double v) ...

	   operator float() const { return value1; }

	   operator double() const { return value2; }


	   float value1;

	   double value2;

   };

   pi = BiValue(3.14) ..

	   probleme sur les arrithmetique

	   pi * pi -> c'est quoi le type





-Est ce que le background ne pourrait pas etre juste une layer dans TM ?


-Pour TiledMap::GeometricObjectPolygon/GeometricObjectPolyline

le bounding box a une taille de 0.!!
cependant, la position correspond au premier point   points[0] (qui est donc toujours 0,0)
ca serait peut etre bien de calculer la bounding box pour de vrai (quitte a deplacer tous les points)





- faire un single include pour lexterieur
<chaos/chaos.h>
ne pas l'utiliser dans chaos, optimiser les dependances'



-TM BoundingBox, oriented BoundingBox pour les collisions

- position initiale du pawn semble foireuse


- camera_class
- player_pawn_class devrait etre editable dans TiledMap (=> LEVEL ou Player Start)
- player_displacement_component_class aussi
- faire une petite doc, meme en .h pour indiquer quelles sont les donn�es disponibles dans TiledMap
- les types mode TMObject ne sont ils pas obsoletes ? (compte tenu du system de class instanciation))

-gestion des inputs merdiques
que faire si on veut utiliser autre chose que les DPAD pour les directions

-reload du player hot ... + souclassement (duplicatat)

-maintenant qu on a un system de classe. revoir peut etre la gestion du HUD. Eviter les choses instanci�es en code. Utiliser a fond le hud.json

-exception Microsoft C++�: boost::filesystem::filesystem_error � l'emplacement de m�moire 0x000000E2E25CB5F0.

- ajouter un autocast au TMObjectCollisionIterator etc...

-autocastable ... erreur de compilation quand rien ne correspond (impossible)

- polygon : orienter les points dans le sens trigo automatiquement ?

-revoir l autorenderToPlayerCameraComponent :
   que se passe t'il avec une safe zone a zero !!

la safe zone devrait etre configurabl


-question: 
   le SetPath(...) devrait il stocker le path apres redirection ?



- dans TMObject, differencier la notion de position et de BoundingBox.
  Il n y a pas de raison pour que la BBox soit centree sur la position


--verifier quels shaders utilsent local_to_world

-se poser la question sur les singletons du system
 pb OpenGL marche avec un etat global, mais si on veut passer sous Vulkan, on ne pourra pas utiliser comme ca des singletons sans reference au Device
	   (ex: Default Materials)

-voir pour la creation d'un CPUMesh

-TMObject revoir ca. Pas de necessit� que cela s'appelle TM (le lien avec TiledMap pourrait peut etre sauter. Resterait un Loader qui utilise TiledMap, point barre)

-pb 
   {
	   DI = GetDrawInterface()

	   DI.GetDynamicMesh().Display() est problematique parce que la draw interface est detruite dans la foul�e alors que le rendu n'est pas effectif'


   }

#endif