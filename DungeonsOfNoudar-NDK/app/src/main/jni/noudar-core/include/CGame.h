#ifndef CGAME_H
#define CGAME_H

namespace Knights {
  class CGame {

  public:
      enum ECommand {
          kUp,
          kRight,
          kDown,
          kLeft,
          kStrafeLeft,
	      kStrafeRight,
	      kFire1,
	      kFire2,
      };
  private:


      std::shared_ptr<CMap> mMap;
      std::shared_ptr<IRenderer> mRenderer;
    bool mIsPlaying;
  public:
    CGame( std::string mapData, std::shared_ptr<IRenderer> renderer );
      ~CGame() = default;
      void endOfTurn(std::shared_ptr<CMap> map);
      void tick();
      bool isPlaying();
  };
}
#endif
