package br.odb.multiplayer;

import br.odb.multiplayer.model.Game;
import br.odb.multiplayer.model.ServerContext;

import javax.servlet.ServletContext;
import javax.servlet.ServletException;
import javax.servlet.http.HttpServlet;
import javax.servlet.http.HttpServletRequest;
import javax.servlet.http.HttpServletResponse;
import java.io.IOException;
import java.io.OutputStream;

/**
 * Servlet implementation class CheckGameStatus
 */
public class GetGameStatus extends HttpServlet {

    /**
     * @see HttpServlet#HttpServlet()
     */
    public GetGameStatus() {
        super();
        // TODO Auto-generated constructor stub
    }

    /**
     * @see HttpServlet#doGet(HttpServletRequest request, HttpServletResponse response)
     */
    protected void doGet(HttpServletRequest request, HttpServletResponse response) throws ServletException, IOException {

        ServerContext context = ServerContext
                .createOrRetrieve((ServletContext) getServletContext());

        OutputStream os = response.getOutputStream();

        String gameId = request.getParameter("gameId");

        if (gameId == null || gameId.isEmpty()) {
            System.out.println("gameId is invalid!");
            return;
        }

        int parsedGameId = Integer.parseInt(gameId);

        System.out.println("gameId is " + gameId + " and playerId is " + request.getParameter("playerId"));

        Game g = context.games.get(parsedGameId);

        if (g != null) {
            g.writeState(os);
        } else {
            System.out.println("game is null!");
        }
    }


    /**
     * @see HttpServlet#doPost(HttpServletRequest request, HttpServletResponse response)
     */
    protected void doPost(HttpServletRequest request, HttpServletResponse response) throws ServletException, IOException {
        // TODO Auto-generated method stub
    }

}
