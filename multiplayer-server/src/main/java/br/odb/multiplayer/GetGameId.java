package br.odb.multiplayer;

import br.odb.multiplayer.model.Game;
import br.odb.multiplayer.model.ServerContext;
import br.odb.multiplayer.model.noudar.NoudarGame;

import javax.servlet.ServletContext;
import javax.servlet.ServletException;
import javax.servlet.http.HttpServlet;
import javax.servlet.http.HttpServletRequest;
import javax.servlet.http.HttpServletResponse;
import java.io.IOException;

/**
 * Servlet implementation class FindGame
 */

public class GetGameId extends HttpServlet {

	class GameIdResponse {
		private int gameId;
		private int playerId;

		@Override
		public String toString() {

			StringBuilder sb = new StringBuilder();

			sb.append("<?xml version='1.0'?>\n<game><gameId>");
			sb.append(gameId);
			sb.append("</gameId><playerId>");
			sb.append(playerId);
			sb.append("</playerId></game>");

			return sb.toString();
		}
	}

	private static final long serialVersionUID = 1L;

	/**
	 * @see HttpServlet#HttpServlet()
	 */
	public GetGameId() {
		super();
	}

	/**
	 * @see HttpServlet#doGet(HttpServletRequest request, HttpServletResponse
	 *      response)
	 */
	protected void doGet(HttpServletRequest request,
			HttpServletResponse response) throws ServletException, IOException {
		
		System.out.println( "GET GAME ID!" );

		ServerContext context = ServerContext
				.createOrRetrieve((ServletContext) getServletContext());

		int playerId;
		GameIdResponse gis = new GameIdResponse();
		Game g = getGameNewOrVacantGame(context);

		playerId = g.addNewPlayer();

		gis.gameId = g.gameId;
		gis.playerId = playerId;

		response.getOutputStream().write(gis.toString().getBytes());

	}

	private Game getGameNewOrVacantGame(ServerContext context	) {

		int bigger = 0;

		Game toReturn;
		// find a existing game pending for new players
		for (Game g : context.games.values()) {
			if (g.players.size() < g.getNumberOfRequiredPlayers()
					&& !g.isTooOld()) {
				System.out.println( "player joining game with id " + ( g.gameId ) );
				return g;
			}

			if (g.gameId > bigger) {
				bigger = g.gameId;
			}
		}

		toReturn = new NoudarGame(bigger + 1);

		System.out.println( "created new game with id " + toReturn.gameId );
		
		context.games.put(toReturn.gameId, toReturn);

		return toReturn;
	}

	/**
	 * @see HttpServlet#doPost(HttpServletRequest request, HttpServletResponse
	 *      response)
	 */
	protected void doPost(HttpServletRequest request,
			HttpServletResponse response) throws ServletException, IOException {

	}
}
