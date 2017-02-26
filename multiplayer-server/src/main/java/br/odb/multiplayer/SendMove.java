package br.odb.multiplayer;

import br.odb.multiplayer.model.Game;
import br.odb.multiplayer.model.ServerContext;

import javax.servlet.ServletException;
import javax.servlet.http.HttpServlet;
import javax.servlet.http.HttpServletRequest;
import javax.servlet.http.HttpServletResponse;
import java.io.IOException;
import java.util.Enumeration;
import java.util.HashMap;

/**
 * Servlet implementation class SendMove
 */
public class SendMove extends HttpServlet {

    /**
     * @see HttpServlet#HttpServlet()
     */
    public SendMove() {
        super();
        // TODO Auto-generated constructor stub
    }

    /**
     * @see HttpServlet#doGet(HttpServletRequest request, HttpServletResponse
     * response)
     */
    protected void doGet(HttpServletRequest request,
                         HttpServletResponse response) throws ServletException, IOException {

        ServerContext context = ServerContext
                .createOrRetrieve(getServletContext());


        System.out.println("GET");

        String parameter = request.getParameter("gameId");
        int gameId = Integer.parseInt(parameter);

        System.out.println("sending move to game with Id " + gameId);
        Game g = context.games.get(gameId);

        if (g.winnerPlayerId != 0) {
            return;
        }

        Enumeration<String> parameterNames = request.getParameterNames();
        HashMap<String, String> params = new HashMap<String, String>();

        while (parameterNames.hasMoreElements()) {

            String paramName = parameterNames.nextElement();
            String paramValues = request.getParameter(paramName);
            params.put(paramName, paramValues);
        }

        g.lastMoveTime = System.currentTimeMillis();
        g.sendMove(params);
        g.checkForGameEnd();
    }

    /**
     * @see HttpServlet#doPost(HttpServletRequest request, HttpServletResponse
     * response)
     */
    protected void doPost(HttpServletRequest request,
                          HttpServletResponse response) throws ServletException, IOException {
        System.out.println("POST");
    }

}
