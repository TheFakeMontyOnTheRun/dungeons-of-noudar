package br.odb.noudar;

import java.io.Serializable;
import java.util.ArrayList;
import java.util.HashMap;
import java.util.List;
import java.util.Map;

import br.odb.droidlib.Tile;
import br.odb.droidlib.Vector2;
import br.odb.menu.GameActivity;

public class GameLevel implements Serializable {

	public static final int MAP_SIZE = 20;
	final private Tile[][] tileMap = new Tile[MAP_SIZE][MAP_SIZE];
	final private ArrayList<Actor> entities = new ArrayList<>();

	final public Map<Vector2, Splat> mSplats = new HashMap<>();


	private int remainingMonsters;
	private int aliveKnightsInCurrentLevel;
	private int mExitedKnights;
	private Knight selectedPlayer;
	private final int mLevelNumber;

	private transient GameViewGLES2.GameRenderer mGameRenderer;
	private transient GameActivity.GameDelegate mGameDelegate;

	public void setDelegates( GameActivity.GameDelegate gameDelegate, GameViewGLES2.GameRenderer gameRenderer ) {
		this.mGameRenderer = gameRenderer;
		this.mGameDelegate = gameDelegate;
	}

	public GameLevel(int[][] map, int levelNumber, GameActivity.GameDelegate gameDelegate, GameViewGLES2.GameRenderer gameRenderer) {

		this.mGameRenderer = gameRenderer;
		this.mGameDelegate = gameDelegate;
		this.mLevelNumber = levelNumber;
		this.aliveKnightsInCurrentLevel = 3;
		buildTilemap(map);
	}

	private void buildTilemap(int[][] map) {
		int[] mapRow;
		Tile tile;

		for (int row = 0; row < map.length; ++row) {
			mapRow = map[row];
			for (int column = 0; column < mapRow.length; ++column) {

				switch (mapRow[column]) {

					case KnightsConstants.BARS:
						tile = new Tile(mapRow[column], GameViewGLES2.ETextures.Bars);
						tile.setKind(mapRow[column]);
						tile.setBlock(true);
						break;

					case KnightsConstants.ARCH:
						tile = new Tile(mapRow[column], GameViewGLES2.ETextures.Arch);
						tile.setBlock(false);
						break;

					case KnightsConstants.BRICKS_BLOOD:
						tile = new Tile(mapRow[column], GameViewGLES2.ETextures.BricksBlood);
						tile.setBlock(true);
						break;

					case KnightsConstants.BRICKS_CANDLES:
						tile = new Tile(mapRow[column], GameViewGLES2.ETextures.BricksCandles);
						tile.setBlock(true);
						break;

					case KnightsConstants.BRICKS:
						tile = new Tile(mapRow[column], GameViewGLES2.ETextures.Bricks);
						tile.setBlock(true);
						break;

					case KnightsConstants.CORNER_LEFT_FAR:
						tile = new Tile(mapRow[column], GameViewGLES2.ETextures.CornerLeftFar);
						tile.setBlock(true);
						break;

					case KnightsConstants.CORNER_LEFT_NEAR:
						tile = new Tile(mapRow[column], GameViewGLES2.ETextures.CornerLeftNear);
						tile.setBlock(true);
						break;

					case KnightsConstants.DOOR:
						tile = new Tile(mapRow[column], GameViewGLES2.ETextures.Exit);
						tile.setBlock(false);
						break;
					case KnightsConstants.BEGIN:
						tile = new Tile(mapRow[column], GameViewGLES2.ETextures.Begin);
						tile.setBlock(true);
						break;
					default:
						tile = new Tile(mapRow[column], GameViewGLES2.ETextures.Grass);
				}
				this.tileMap[row][column] = tile;

				int kind = tile.getKind();

				switch (kind) {

					case KnightsConstants.SPAWNPOINT_BAPHOMET:
						addEntity(new Baphomet(), column, row);
						break;
					case KnightsConstants.SPAWNPOINT_BULL:
						addEntity(new BullKnight(), column, row);
						break;
					case KnightsConstants.SPAWNPOINT_TURTLE:
						addEntity(new TurtleKnight(), column, row);
						break;
					case KnightsConstants.SPAWNPOINT_EAGLE:
						addEntity(new EagleKnight(), column, row);
						break;
					case KnightsConstants.SPAWNPOINT_CUCO:
						addEntity(new Cuco(), column, row);
						break;
					case KnightsConstants.SPAWNPOINT_MOURA:
						addEntity(new Moura(), column, row);
						break;
					case KnightsConstants.SPAWNPOINT_DEVIL:
						addEntity(new Demon(), column, row);
						break;
				}
			}
		}

		updateCounters();
	}

	public synchronized void  tick() {
		Monster m;

		for (Actor a : entities) {
			a.notifyEndOfTurn();

			if ( a.isAlive() ) {
				if ( a == selectedPlayer ) {
					a.setActiveStance();
				} else {
					a.setRestedStance();
				}
			} else {
				a.setAsDead();
			}

			if (a.isAlive() && (a instanceof Monster)) {
				m = (Monster) a;
				m.updateTarget(this);
			}
		}

		updateCounters();
	}

	public void updateSplats(long ms) {
		List<Vector2> toRemove = new ArrayList<>();

		for (Vector2 pos : mSplats.keySet()) {
			Splat splat = mSplats.get(pos);
			splat.update(ms);

			if (splat.isFinished()) {
				toRemove.add(pos);
			}
		}


		for (Vector2 pos : toRemove) {
			mSplats.remove(pos);
		}

		if ( !toRemove.isEmpty()) {
			mGameRenderer.setNeedsUpdate();
		}
	}

	private void addEntity(Actor actor, int x, int y) {
		entities.add(actor);
		tileMap[y][x].setOccupant(actor);
		actor.setPosition(new Vector2(x, y));
	}

	public Tile getTile(Vector2 position) {
		return this.tileMap[(int) position.y][(int) position.x];
	}

	public boolean validPositionFor(Actor actor) {

		if ( !actor.isAlive() ) {
			return false;
		}

		int row, column;
		row = (int) actor.getPosition().y;
		column = (int) actor.getPosition().x;

		if (tileMap[row][column].isBlock()) {
			return false;
		}

		if ((tileMap[row][column].getOccupant() instanceof Actor)
				&& !((Actor) tileMap[row][column].getOccupant()).isAlive()) {
			return true;
		}

		if ((actor instanceof Monster)
				&& ( tileMap[row][column].getKind() == KnightsConstants.BEGIN || tileMap[row][column].getKind() == KnightsConstants.DOOR ) ) {
			return false;
		}

		if ((tileMap[row][column].getOccupant() instanceof Knight)
				&& ((Knight) tileMap[row][column].getOccupant()).hasExited) {
			return true;
		}

		return !(tileMap[row][column].getOccupant() instanceof Actor);
	}

	private Actor getActorAt(int x, int y) {

		if (tileMap[y][x].getOccupant() instanceof Actor)
			return ((Actor) tileMap[y][x].getOccupant());
		else
			return null;
	}

	public void battle(Actor attacker, Actor defendant) {

		Vector2 pos;

		createSplatAt(attacker.getPosition());
		createSplatAt(defendant.getPosition());

		attacker.attack(defendant);
		defendant.attack(attacker);

		if (!attacker.isAlive()) {
			pos = attacker.getPosition();
			tileMap[(int) pos.y][(int) pos.x].setOccupant(null);
		}


		if (!defendant.isAlive()) {
			pos = defendant.getPosition();
			tileMap[(int) pos.y][(int) pos.x].setOccupant(null);
		}
	}

	void createSplatAt(Vector2 pos) {
		mSplats.put(pos, new Splat());
	}

	public Actor getActorAt(Vector2 position) {

		return getActorAt((int) position.x, (int) position.y);
	}

	public Knight[] getKnights() {
		List<Knight> knights_filtered = new ArrayList<>();

		for (Actor a : entities) {
			if (a instanceof Knight && a.isAlive() && !((Knight) a).hasExited) {
				knights_filtered.add((Knight) a);
			}
		}

		Knight[] knights = new Knight[knights_filtered.size()];
		return knights_filtered.toArray(knights);
	}

	public synchronized int getMonsters() {
		return remainingMonsters;
	}

	public synchronized int getTotalAvailableKnights() {
		return this.aliveKnightsInCurrentLevel;
	}

	private boolean isBlockAt(int x, int y) {
		return tileMap[y][x].isBlock();
	}

	public boolean canMove(Actor actor, GameActivity.Direction direction) {
		Vector2 position = actor.getPosition().add(direction.getOffsetVector());

		return !isBlockAt((int) position.x, (int) position.y);
	}

	public boolean canAttack(Actor actor, GameActivity.Direction direction) {
		Vector2 position = actor.getPosition().add(direction.getOffsetVector());
		return getActorAt((int) position.x, (int) position.y) instanceof Monster;
	}

	public boolean needsUpdate() {
		return !mSplats.keySet().isEmpty();
	}


	public int getLevelNumber() {
		return this.mLevelNumber;
	}

	public synchronized int getTotalExitedKnights() {
		return mExitedKnights;
	}

	public void selectDefaultKnight() {
		Knight newSelected = null;

		for ( Knight k : getKnights()) {
			if ( k.isAlive() && !k.hasExited) {
				newSelected = k;
			}
		}

		setSelectedPlayer( newSelected);
	}


	public Knight getSelectedPlayer() {
		return selectedPlayer;
	}

	public void setSelectedPlayer(Knight knight) {
		this.selectedPlayer = knight;
	}

	public void cycleSelectNextKnight() {
		int index = 0;
		Knight[] knights = getKnights();

		for (Knight k : knights) {
			if (selectedPlayer == k) {
				selectedPlayer = knights[((index + 1) % (knights.length))];
			} else {
				++index;
			}
		}
	}

	public void updateCounters() {
		int aliveMonsters = 0;
		int aliveKnights = 0;
		int exitedKnights = 0;

		for (Actor a : entities) {

			if (a.isAlive()) {
				if (a instanceof Monster) {
					++aliveMonsters;
				} else if (!(((Knight) a).hasExited)) {
					++aliveKnights;
				} else {
					++exitedKnights;
				}
			}
		}

		remainingMonsters = aliveMonsters;
		aliveKnightsInCurrentLevel = aliveKnights;
		mExitedKnights = exitedKnights;
	}

	public synchronized void handleCommand(GameViewGLES2.KB key) {

			Tile loco = getTile(getSelectedPlayer().getPosition());

			getSelectedPlayer().checkpointPosition();

			switch (key) {
				case UP:
					getSelectedPlayer().act(Actor.Actions.MOVE_UP);
					break;
				case DOWN:
					getSelectedPlayer().act(Actor.Actions.MOVE_DOWN);
					break;
				case LEFT:
					getSelectedPlayer().act(Actor.Actions.MOVE_LEFT);
					break;
				case RIGHT:
					getSelectedPlayer().act(Actor.Actions.MOVE_RIGHT);
					break;
				case ROTATE_LEFT:
					mGameRenderer.cameraRotateLeft();
					mGameDelegate.onKnightChanged();
					return;
				case ROTATE_RIGHT:
					mGameRenderer.cameraRotateRight();
					mGameDelegate.onKnightChanged();
					return;

				case CYCLE_CURRENT_KNIGHT:
					cycleSelectNextKnight();
					mGameDelegate.onKnightChanged();
					mGameRenderer.setNeedsUpdate();
					return;

			}

			if (!validPositionFor(getSelectedPlayer())) {

				if (getActorAt(getSelectedPlayer().getPosition()) instanceof Monster ) {
					battle(getSelectedPlayer(),	getActorAt(getSelectedPlayer().getPosition()));
				}

				getSelectedPlayer().undoMove();
			} else {
				loco.setOccupant(null);
				loco = getTile(getSelectedPlayer().getPosition());
				loco.setOccupant(getSelectedPlayer());
			}

			if ( getSelectedPlayer() != null && getSelectedPlayer().isAlive() && loco.getKind() == KnightsConstants.DOOR) {
				selectedPlayerHasExited();
			}

			tick();
			updateCounters();

			//player could have died during a NPC attack
			if ( getSelectedPlayer() != null && !getSelectedPlayer().isAlive()) {
				selectedPlayerHasDied();
			}

			updateCounters();
			mGameRenderer.setNeedsUpdate();
			mGameDelegate.onTurnEnded();
		}


	private void selectedPlayerHasExited() {
		getSelectedPlayer().setAsExited();

		if ( getTotalAvailableKnights() > 1 ) {
			mGameRenderer.displayKnightEnteredDoorMessage();
		}

		if ( getTotalAvailableKnights() > 0) {
			selectDefaultKnight();
		}
	}

	private void selectedPlayerHasDied() {

		if (getTotalAvailableKnights() == 0 && getTotalExitedKnights() == 0 ) {
			mGameRenderer.fadeOut();
			mGameDelegate.onGameOver();
		} else {
			mGameRenderer.displayKnightIsDeadMessage();

			if ( getTotalAvailableKnights() > 0) {
				selectDefaultKnight();
			}
		}
	}
}
