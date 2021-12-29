
# pragma once
# include <Siv3D.hpp>

// Photono SDK クラスの前方宣言
namespace ExitGames
{
	namespace LoadBalancing
	{
		class Listener;
		class Client;
	}
}

namespace s3d
{
	namespace NetworkSystem
	{
		/// @brief 暗号化された Photon アプリケーション ID を復号します。
		/// @param encryptedPhotonAppID 暗号化された Photon アプリケーション ID
		/// @return 復号された Photon アプリケーション ID
		inline String DecryptPhotonAppID(StringView encryptedPhotonAppID)
		{
			// [Siv3D ToDo] あとで実装
			return String{ encryptedPhotonAppID };
		}

		inline constexpr int32 NoRandomMatchFound = (0x7FFF - 7);
	}

	class SivPhoton
	{
	public:

		/// @brief SivPhoton を作成します。
		/// @param secretPhotonAppID Photon アプリケーション ID
		/// @param photonAppVersion アプリケーションのバージョンです。
		/// @remark アプリケーションバージョンが異なる SivPhoton とは通信できません。
		SivPhoton(StringView secretPhotonAppID, StringView photonAppVersion);

		virtual ~SivPhoton();

		/// @brief Photon サーバへの接続を試みます。
		/// @param userName ユーザ名
		void connect(StringView userName = U"", const Optional<String>& defaultRoomName = unspecified);

		void disconnect();

		/// @brief サーバーといい感じにします。
		/// @remark 6 秒間以上この関数を呼ばないと自動的に切断されます。
		void update();

		/// @brief ルーム
		/// @param maxPlayers ルームの最大人数
		/// @remark 最大 255, 無料の Photon アカウントの場合は 20
		void opJoinRandomRoom(int32 maxPlayers);

		void opJoinRoom(StringView roomName, bool rejoin = false);

		void opCreateRoom(StringView roomName, int32 maxPlayers);

		void opLeaveRoom();

		template <class T>
		void opRaiseEvent(uint8 eventCode, const T& value);

		void opRaiseEvent(uint8 eventCode, const int32 value);

		void opRaiseEvent(uint8 eventCode, const double value);

		void opRaiseEvent(uint8 eventCode, const float value);

		void opRaiseEvent(uint8 eventCode, const bool value);

		void opRaiseEvent(uint8 eventCode, const StringView value);

		void opRaiseEvent(uint8 eventCode, const Array<int32>& value);

		void opRaiseEvent(uint8 eventCode, const Array<double>& value);

		void opRaiseEvent(uint8 eventCode, const Array<float>& value);

		void opRaiseEvent(uint8 eventCode, const Array<bool>& value);

		void opRaiseEvent(uint8 eventCode, const Array<String>& value);

		void opRaiseEvent(uint8 eventCode, const Grid<int32>& value);

		void opRaiseEvent(uint8 eventCode, const Grid<double>& value);

		void opRaiseEvent(uint8 eventCode, const Grid<float>& value);

		void opRaiseEvent(uint8 eventCode, const Grid<bool>& value);

		void opRaiseEvent(uint8 eventCode, const Grid<String>& value);

		//template <class Type>
		//void opRaiseEvent(uint8 eventCode, const HashTable<uint8, Type>& parameters);

		/// @brief サーバに接続したときのユーザ名を返します。
		/// @return ユーザ名
		[[nodiscard]]
		String getName() const;

		/// @brief サーバに接続したときのユーザ ID (ユーザ名 + タイムスタンプ）を返します。
		/// @return ユーザ ID
		[[nodiscard]]
		String getUserID() const;

		/// @brief 存在するルーム名の一覧を返します。
		/// @remark 自分でルームを作成してそれに参加すると表示されないっぽい (?)
		/// @return 存在するルーム名の一覧
		[[nodiscard]]
		Array<String> getRoomNameList() const;

		/// @brief 自分がルームに参加しているかを返します。
		/// @return ルームに参加している場合 true, それ以外の場合は false
		[[nodiscard]]
		bool isInRoom() const;

		[[nodiscard]]
		String getCurrentRoomName() const;

		[[nodiscard]]
		int32 getPlayerCountInCurrentRoom() const;

		[[nodiscard]]
		int32 getMaxPlayersInCurrentRoom() const;

		[[nodiscard]]
		bool getIsOpenInCurrentRoom() const;

		[[nodiscard]]
		bool getIsVisibleInCurrentRoom() const;

		void setIsOpenInCurrentRoom(bool isOpen);

		void setIsVisibleInCurrentRoom(bool isVisible);

		[[nodiscard]]
		int32 getCountGamesRunning() const;

		[[nodiscard]]
		int32 getCountPlayersIngame() const;

		[[nodiscard]]
		int32 getCountPlayersOnline() const;

		/// @brief ルーム内でのプレイヤー ID を返します。
		/// @return ルーム内でのプレイヤー ID, ルームに参加していない場合は none
		[[nodiscard]]
		Optional<int32> localPlayerID() const;

		/// @brief 自分がマスタークライアントであるかを返します。
		/// @return マスタークライアントである場合 true, それ以外の場合は false
		[[nodiscard]]
		bool isMasterClient() const;

		/// @brief ルーム内でのプレイヤー ID を返します。(?)
		/// @return ルーム内でのプレイヤー ID (?)
		[[nodiscard]]
		int32 getNumber() const;

		[[nodiscard]]
		bool isUsePhoton() const noexcept;

		virtual void connectionErrorReturn(int32 errorCode);

		virtual void connectReturn(int32 errorCode, const String& errorString, const String& region, const String& cluster);

		virtual void disconnectReturn();

		virtual void leaveRoomReturn(int32 errorCode, const String& errorString);

		virtual void joinRandomRoomReturn(int32 localPlayerID, int32 errorCode, const String& errorString);

		virtual void joinRoomReturn(int32 localPlayerID, int32 errorCode, const String& errorString);

		virtual void joinRoomEventAction(int32 localPlayerID, const Array<int32>& playerIDs, bool isSelf);

		virtual void leaveRoomEventAction(int32 playerID, bool isInactive);

		virtual void createRoomReturn(int32 localPlayerID, int32 errorCode, const String& errorString);

		virtual void customEventAction(int32 playerID, int32 eventCode, const int32 eventContent);

		virtual void customEventAction(int32 playerID, int32 eventCode, const double eventContent);

		virtual void customEventAction(int32 playerID, int32 eventCode, const float eventContent);

		virtual void customEventAction(int32 playerID, int32 eventCode, const bool eventContent);

		virtual void customEventAction(int32 playerID, int32 eventCode, const String& eventContent);

		virtual void customEventAction(int32 playerID, int32 eventCode, const Array<int32>& eventContent);

		virtual void customEventAction(int32 playerID, int32 eventCode, const Array<double>& eventContent);

		virtual void customEventAction(int32 playerID, int32 eventCode, const Array<float>& eventContent);

		virtual void customEventAction(int32 playerID, int32 eventCode, const Array<bool>& eventContent);

		virtual void customEventAction(int32 playerID, int32 eventCode, const Array<String>& eventContent);

		virtual void customEventAction(int32 playerID, int32 eventCode, const Grid<int32>& eventContent);

		virtual void customEventAction(int32 playerID, int32 eventCode, const Grid<double>& eventContent);

		virtual void customEventAction(int32 playerID, int32 eventCode, const Grid<float>& eventContent);

		virtual void customEventAction(int32 playerID, int32 eventCode, const Grid<bool>& eventContent);

		virtual void customEventAction(int32 playerID, int32 eventCode, const Grid<String>& eventContent);

		virtual void customEventAction(int32 playerID, int32 eventCode, const Point& eventContent);

		virtual void customEventAction(int32 playerID, int32 eventCode, const Vec2& eventContent);

		virtual void customEventAction(int32 playerID, int32 eventCode, const Rect& eventContent);

		virtual void customEventAction(int32 playerID, int32 eventCode, const Circle& eventContent);

		virtual void customEventAction(int32 playerID, int32 eventCode, const Array<Point>& eventContent);

		virtual void customEventAction(int32 playerID, int32 eventCode, const Array<Vec2>& eventContent);

		virtual void customEventAction(int32 playerID, int32 eventCode, const Array<Rect>& eventContent);

		virtual void customEventAction(int32 playerID, int32 eventCode, const Array<Circle>& eventContent);

		virtual void customEventAction(int32 playerID, int32 eventCode, const Grid<Point>& eventContent);

		virtual void customEventAction(int32 playerID, int32 eventCode, const Grid<Vec2>& eventContent);

		virtual void customEventAction(int32 playerID, int32 eventCode, const Grid<Rect>& eventContent);

		virtual void customEventAction(int32 playerID, int32 eventCode, const Grid<Circle>& eventContent);

	protected:

		String m_defaultRoomName;

	private:

		class SivPhotonDetail;

		std::unique_ptr<ExitGames::LoadBalancing::Listener> m_listener;

		std::unique_ptr<ExitGames::LoadBalancing::Client> m_client;

		bool m_isUsePhoton = false;

		/// @brief リスナーの参照を返します。
		/// @return リスナーの参照
		[[nodiscard]]
		ExitGames::LoadBalancing::Client& getClient();
	};
}
