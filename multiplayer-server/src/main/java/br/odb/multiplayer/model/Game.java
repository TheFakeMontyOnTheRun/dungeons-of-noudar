package br.odb.multiplayer.model;

import java.io.OutputStream;
import java.util.ArrayList;
import java.util.HashMap;

public abstract class Game {
	
	public final HashMap< Integer, Player > players = new HashMap<>();
	public final int type;
	public final int gameId;
	public int winnerPlayerId;
//	public int currentPlayerId;
	public long lastMoveTime;
	public static final long TIME_LIMIT = 2 * 60 * 1000;
	
	public Game( int gameId, int gameType ) {
		this.type = gameType;
		this.gameId = gameId;
		this.winnerPlayerId = 0;
//		this.currentPlayerId = 0;
		lastMoveTime = System.currentTimeMillis();
	}
	
	public int addNewPlayer() {
		lastMoveTime = System.currentTimeMillis();
		Player player = makeNewPlayer(); 
		players.put( player.playerId, player );
		System.out.println( "player added with id " + player.playerId + " on game with id " + gameId  );
//		currentPlayerId = player.playerId;
		
		return player.playerId;
	}
	
	public void setTheNextPlayerAsCurrent() {
		ArrayList<Integer> playerIds = new ArrayList<Integer>();
		playerIds.addAll(players.keySet());
//		int indexOfCurrentIndex = playerIds.indexOf( currentPlayerId );
//		currentPlayerId = playerIds.get( (indexOfCurrentIndex + 1) % playerIds.size() );
	}

	public Player makeNewPlayer() {
		System.out.println( "creating a new player" );
		return new Player( players.size() + 1, gameId );
	}

	public abstract void checkForGameEnd() ;

	public abstract void sendMove(HashMap<String, String> params);

	public abstract void writeState(OutputStream os);
	
	public abstract int getNumberOfRequiredPlayers();

	public boolean isTooOld() {
		return false;//return ( System.currentTimeMillis() - lastMoveTime ) > TIME_LIMIT;
	}
}
