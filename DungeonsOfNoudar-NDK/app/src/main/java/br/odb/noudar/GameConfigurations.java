package br.odb.noudar;

public class GameConfigurations {

    public static GameConfigurations getInstance() {

        if (instance == null) {
            instance = new GameConfigurations();
        }

        return instance;
    }

    private GameSession currentGameSession = new GameSession();
    private static GameConfigurations instance = null;

	private GameConfigurations() {
	}

    public GameSession getCurrentGameSession() {
        return currentGameSession;
    }
}
