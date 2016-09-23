package br.odb.noudar;

public class GameConfigurations {

    public static GameConfigurations getInstance() {

        if (instance == null) {
            instance = new GameConfigurations();
        }

        return instance;
    }

    private GameSession currentGameSession;
    private static GameConfigurations instance = null;

	private GameConfigurations() {
		GameSession session = getCurrentGameSession();
		if ( session != null ) {
			session.resetScore();
		}
	}

    public GameSession getCurrentGameSession() {
        return currentGameSession;
    }

    public void startNewSession( int commulatedScore) {
        currentGameSession = new GameSession();
        currentGameSession.addtoScore(commulatedScore);
    }
}
