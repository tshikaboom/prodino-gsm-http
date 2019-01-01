/* SPDX-License-Identifier: MIT */

#include "config.h"

#ifdef CONFIG_CITATION

#define ARRAY_LEN 17

const char* citation[] = {
  "Qui vole un boeuf est vachement musclé.",
  "Mouette qui pète, gare à la tempête... ",
  "Pingouins dans les champs, hiver méchant...",
  "Il n'y a qu'ensemble qu'on sera plusieurs.",
  "Les 5 symptômes de la paresse :<br />  1.",
  "La vitamine C... mais elle ne dira rien.",
  "Bon je vous laisse, je vais faire une machine !",
  "Donner c'est donner et repeindre ses volets.",
  "Qui fait pipi contre le vent, ... se rince les dents",
  "Si t'es fière d'être Blanche Neige, tape dans tes nains.",
  "Noël au balcon, enrhumé comme un con.",
  "Qui pisse loin ménage ses pompes.",
  "Plus il y de fous, moins il y a de riz !",
  "Chassez le naturiste il revient au bungalow.",
  "Quand tu marches vers le Nord, tu as le sudoku.",
  "Vous ne m'avez pas cru, vous m'aurez cuite !",
  "Elle a frit, elle a tout compris!"
};

const char* auteur[] = {
  "- Mao Tsetung",
  "- Blaise Pascal",
  "- Winston Churchill",
  "- George Boole",
  "- Félix Faure",
  "- Frank Columbo",
  "- Leonard de Vinci",
  "- Alfred Hitchcock",
  "- Lao Tseu",
  "- Mimi Mathy",
  "- Pamela Anderson",
  "- Christian Louboutin",
  "- Nicolas Fonsat",
  "- Dominique Strauss-Kahn",
  "- Sun Tzu",
  "- Jeanne d'Arc",
  "- Jeanne d'Arc"
};


void HTTP200Citation(EthernetClient client)
{
  long rand = random(100) % ARRAY_LEN;

  PR_DEBUG("Serving citation #");
  PR_DEBUGLN(rand);

  // send a standard http response header
  client.println("HTTP/1.1 200 OK");
  client.println("Content-Type: text/html");
  client.println("Connection: close");  // the connection will be closed after completion of the response
  client.println("Refresh: 5");  // refresh the page automatically every 5 sec
  client.print("\r\n");
  client.println("<!DOCTYPE HTML>");
  client.println("<html>");
  client.println("<head>");
  client.println("<meta charset=\"utf-8\">");
  client.println("</head>");

  client.println(citation[rand]);
  client.println("<br />");
  client.println(auteur[rand]);

  client.println("<br />");
  client.println("</html>");
}

#endif // CONFIG_CITATION
