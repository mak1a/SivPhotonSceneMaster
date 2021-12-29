
# pragma once
# include "NetworkSystem.hpp"

// SivPhotonSceneMasterの宣言
namespace s3d::NetworkSystem
{
	template <class State, class Data> class SivPhotonSceneMaster;

	/// @brief シーンのインタフェース
	/// @tparam State シーンを区別するキーの型
	/// @tparam Data シーン間で共有するデータの型
	template <class State, class Data>
	class IScene : Uncopyable
	{
	public:

		using State_t = State;

		using Data_t = Data;

		struct InitData
		{
			State_t state;

			std::shared_ptr<Data_t> _s;

			SivPhotonSceneMaster<State_t, Data_t>* _m;

			SIV3D_NODISCARD_CXX20
				InitData() = default;

			SIV3D_NODISCARD_CXX20
				InitData(const State_t& _state, const std::shared_ptr<Data_t>& data, SivPhotonSceneMaster<State_t, Data_t>* manager);
		};

	public:

		SIV3D_NODISCARD_CXX20
			explicit IScene(const InitData& init);

		virtual ~IScene() = default;

		/// @brief フェードイン時の更新処理です。
		/// @param t フェードインの進度 [0.0, 1.0]
		virtual void updateFadeIn([[maybe_unused]] double t) {}

		/// @brief 通常時の更新処理です。
		virtual void update() {}

		/// @brief フェードアウト時の更新処理です。
		/// @param t フェードアウトの進度 [0.0, 1.0]
		virtual void updateFadeOut([[maybe_unused]] double t) {}

		/// @brief 通常時の描画処理です。
		virtual void draw() const {}

		/// @brief フェードイン時の描画処理です。
		/// @param t フェードインの進度 [0.0, 1.0]
		virtual void drawFadeIn(double t) const;

		/// @brief フェードアウト時の描画処理です。
		/// @param t フェードアウトの進度 [0.0, 1.0]
		virtual void drawFadeOut(double t) const;

	public:
		// 通信用のあれこれ
		/// @brief Photon サーバへの接続を試みます。
		/// @param userName ユーザ名
		void connect(StringView userName = U"", const Optional<String>& defaultRoomName = unspecified);

		void disconnect();

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

		void setIsVisibleInCurrentRoom(const bool isVisible);

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

		virtual void connectionErrorReturn(int32 errorCode);

		virtual void connectReturn(int32 errorCode, const String& errorString, const String& region, const String& cluster);

		virtual void disconnectReturn();

		virtual void leaveRoomReturn(int32 errorCode, const String& errorString);

		virtual void joinRandomRoomReturn(int32 localPlayerID, int32 errorCode, const String& errorString);

		virtual void joinRoomReturn(int32 localPlayerID, int32 errorCode, const String& errorString);

		virtual void joinRoomEventAction(int32 localPlayerID, const Array<int32>& playerIDs, bool isSelf);

		virtual void leaveRoomEventAction(int32 playerID, bool isInactive);

		virtual void createRoomReturn(int32 localPlayerID, int32 errorCode, const String& errorString);

		virtual void customEventAction(const int32 playerID, const int32 eventCode, const int32 eventContent);

		virtual void customEventAction(const int32 playerID, const int32 eventCode, const double eventContent);

		virtual void customEventAction(const int32 playerID, const int32 eventCode, const float eventContent);

		virtual void customEventAction(const int32 playerID, const int32 eventCode, const bool eventContent);

		virtual void customEventAction(const int32 playerID, const int32 eventCode, const String& eventContent);

		virtual void customEventAction(const int32 playerID, const int32 eventCode, const Array<int32>& eventContent);

		virtual void customEventAction(const int32 playerID, const int32 eventCode, const Array<double>& eventContent);

		virtual void customEventAction(const int32 playerID, const int32 eventCode, const Array<float>& eventContent);

		virtual void customEventAction(const int32 playerID, const int32 eventCode, const Array<bool>& eventContent);

		virtual void customEventAction(const int32 playerID, const int32 eventCode, const Array<String>& eventContent);

		virtual void customEventAction(const int32 playerID, const int32 eventCode, const Grid<int32>& eventContent);

		virtual void customEventAction(const int32 playerID, const int32 eventCode, const Grid<double>& eventContent);

		virtual void customEventAction(const int32 playerID, const int32 eventCode, const Grid<float>& eventContent);

		virtual void customEventAction(const int32 playerID, const int32 eventCode, const Grid<bool>& eventContent);

		virtual void customEventAction(const int32 playerID, const int32 eventCode, const Grid<String>& eventContent);

		virtual void customEventAction(const int32 playerID, const int32 eventCode, const Point& eventContent);

		virtual void customEventAction(const int32 playerID, const int32 eventCode, const Vec2& eventContent);

		virtual void customEventAction(const int32 playerID, const int32 eventCode, const Rect& eventContent);

		virtual void customEventAction(const int32 playerID, const int32 eventCode, const Circle& eventContent);

		virtual void customEventAction(const int32 playerID, const int32 eventCode, const Array<Point>& eventContent);

		virtual void customEventAction(const int32 playerID, const int32 eventCode, const Array<Vec2>& eventContent);

		virtual void customEventAction(const int32 playerID, const int32 eventCode, const Array<Rect>& eventContent);

		virtual void customEventAction(const int32 playerID, const int32 eventCode, const Array<Circle>& eventContent);

		virtual void customEventAction(const int32 playerID, const int32 eventCode, const Grid<Point>& eventContent);

		virtual void customEventAction(const int32 playerID, const int32 eventCode, const Grid<Vec2>& eventContent);

		virtual void customEventAction(const int32 playerID, const int32 eventCode, const Grid<Rect>& eventContent);

		virtual void customEventAction(const int32 playerID, const int32 eventCode, const Grid<Circle>& eventContent);

	protected:

		/// @brief 現在のステートのキーを取得します。
		/// @return 現在のステートのキー
		[[nodiscard]]
		const State_t& getState() const;

		/// @brief 共有データへの参照を取得します。
		/// @return 共有データへの参照
		template <class DataType = Data, std::enable_if_t<not std::disjunction_v<std::is_array<DataType>, std::is_void<DataType>>>* = nullptr>
		[[nodiscard]]
		DataType& getData() const;

		/// @brief シーンの変更をリクエストします。
		/// @param state 次のシーンのキー
		/// @param transitionTime フェードイン・アウトの時間
		/// @param crossFade クロスフェードを有効にするか
		/// @return シーンの変更が開始される場合 true, それ以外の場合は false
		bool changeScene(const State_t& state, const Duration& transitionTime = Duration{ 2.0 }, CrossFade crossFade = CrossFade::No);

		/// @brief シーンの変更をリクエストします。
		/// @param state 次のシーンのキー
		/// @param transitionTimeMillisec フェードイン・アウトの時間（ミリ秒）
		/// @param crossFade クロスフェードを有効にするか
		/// @return シーンの変更が開始される場合 true, それ以外の場合は false
		bool changeScene(const State_t& state, int32 transitionTimeMillisec, CrossFade crossFade = CrossFade::No);

		/// @brief エラーの発生を通知します。
		/// @remark この関数を呼ぶと、以降のこのシーンを管理するクラスの `SivPhotonSceneMaster::update()` が false を返します。
		void notifyError();

	private:

		State_t m_state;

		std::shared_ptr<Data_t> m_data;

		SivPhotonSceneMaster<State_t, Data_t>* m_manager;
	};

	/// @brief シーン遷移管理
	/// @tparam State シーンを区別するキーの型
	/// @tparam Data シーン間で共有するデータの型
	template <class State, class Data = void>
	class SivPhotonSceneMaster : public SivPhoton
	{
	public:

		/// @brief シーンのインタフェース型
		using Scene = IScene<State, Data>;

		/// @brief シーン管理を初期化します。
		SIV3D_NODISCARD_CXX20
			SivPhotonSceneMaster(StringView secretPhotonAppID, StringView photonAppVersion);

		/// @brief シーン管理を初期化します。
		/// @param data 共有データ
		SIV3D_NODISCARD_CXX20
			explicit SivPhotonSceneMaster(const std::shared_ptr<Data>& data, StringView secretPhotonAppID, StringView photonAppVersion);

		/// @brief シーンを登録します。
		/// @tparam SceneType シーンの型
		/// @param state シーンのキー
		/// @return シーンの登録に成功した場合 true, それ以外の場合は false
		template <class SceneType>
		SivPhotonSceneMaster& add(const State& state);

		/// @brief 最初のシーンを初期化します。
		/// @param state 最初のシーン
		/// @return 初期化に成功した場合 true, それ以外の場合は false
		bool init(const State& state);

		/// @brief 現在のシーンの更新処理のみを行います。
		/// @remark 通常はこの関数は使用しません。
		/// @return シーンの更新処理に成功した場合 true, それ以外の場合は false
		bool updateScene();

		/// @brief 現在のシーンの描画処理のみを行います。
		/// @remark 通常はこの関数は使用しません。
		void drawScene() const;

		/// @brief 現在のシーンの更新処理と描画処理を行います。
		/// @return シーンの更新処理に成功した場合 true, それ以外の場合は false
		bool update();

		/// @brief 共有データを取得します。
		/// @return 共有データへのポインタ
		[[nodiscard]]
		std::shared_ptr<Data> get() noexcept;

		/// @brief 共有データを取得します。
		/// @return 共有データへのポインタ
		[[nodiscard]]
		const std::shared_ptr<const Data> get() const noexcept;

		/// @brief シーンを変更します。
		/// @param state 次のシーンのキー
		/// @param transitionTime フェードイン・アウトの時間
		/// @param crossFade ロスフェードを有効にするか
		/// @return シーンの変更が開始される場合 true, それ以外の場合は false
		bool changeScene(const State& state, const Duration& transitionTime = Duration{ 2.0 }, CrossFade crossFade = CrossFade::No);

		/// @brief シーンを変更します。
		/// @param state 次のシーンのキー
		/// @param transitionTimeMillisec フェードイン・アウトの時間（ミリ秒）
		/// @param crossFade クロスフェードを有効にするか
		/// @return シーンの変更が開始される場合 true, それ以外の場合は false
		bool changeScene(const State& state, int32 transitionTimeMillisec, CrossFade crossFade = CrossFade::No);

		/// @brief デフォルトのフェードイン・アウトに使う色を設定します。
		/// @param color デフォルトのフェードイン・アウトに使う色
		/// @return *this
		SivPhotonSceneMaster& setFadeColor(const ColorF& color) noexcept;

		/// @brief デフォルトのフェードイン・アウトに使う色を返します。
		/// @return デフォルトのフェードイン・アウトに使う色
		[[nodiscard]]
		const ColorF& getFadeColor() const noexcept;

		/// @brief エラーの発生を通知します。
		/// @return この関数を呼ぶと、以降のこのクラスの `SivPhotonSceneMaster::update()` が false を返します。
		void notifyError() noexcept;

	private:
		// 通信関係のあれこれ
		void connectionErrorReturn(int32 errorCode);

		void connectReturn(int32 errorCode, const String& errorString, const String& region, const String& cluster);

		void disconnectReturn();

		void leaveRoomReturn(int32 errorCode, const String& errorString);

		void joinRandomRoomReturn(int32 localPlayerID, int32 errorCode, const String& errorString);

		void joinRoomReturn(int32 localPlayerID, int32 errorCode, const String& errorString);

		void joinRoomEventAction(int32 localPlayerID, const Array<int32>& playerIDs, bool isSelf);

		void leaveRoomEventAction(int32 playerID, bool isInactive);

		void createRoomReturn(int32 localPlayerID, int32 errorCode, const String& errorString);

		void customEventAction(const int32 playerID, const int32 eventCode, const int32 eventContent);

		void customEventAction(const int32 playerID, const int32 eventCode, const double eventContent);

		void customEventAction(const int32 playerID, const int32 eventCode, const float eventContent);

		void customEventAction(const int32 playerID, const int32 eventCode, const bool eventContent);

		void customEventAction(const int32 playerID, const int32 eventCode, const String& eventContent);

		void customEventAction(const int32 playerID, const int32 eventCode, const Array<int32>& eventContent);

		void customEventAction(const int32 playerID, const int32 eventCode, const Array<double>& eventContent);

		void customEventAction(const int32 playerID, const int32 eventCode, const Array<float>& eventContent);

		void customEventAction(const int32 playerID, const int32 eventCode, const Array<bool>& eventContent);

		void customEventAction(const int32 playerID, const int32 eventCode, const Array<String>& eventContent);

		void customEventAction(const int32 playerID, const int32 eventCode, const Grid<int32>& eventContent);

		void customEventAction(const int32 playerID, const int32 eventCode, const Grid<double>& eventContent);

		void customEventAction(const int32 playerID, const int32 eventCode, const Grid<float>& eventContent);

		void customEventAction(const int32 playerID, const int32 eventCode, const Grid<bool>& eventContent);

		void customEventAction(const int32 playerID, const int32 eventCode, const Grid<String>& eventContent);

		void customEventAction(const int32 playerID, const int32 eventCode, const Point& eventContent);

		void customEventAction(const int32 playerID, const int32 eventCode, const Vec2& eventContent);

		void customEventAction(const int32 playerID, const int32 eventCode, const Rect& eventContent);

		void customEventAction(const int32 playerID, const int32 eventCode, const Circle& eventContent);

		void customEventAction(const int32 playerID, const int32 eventCode, const Array<Point>& eventContent);

		void customEventAction(const int32 playerID, const int32 eventCode, const Array<Vec2>& eventContent);

		void customEventAction(const int32 playerID, const int32 eventCode, const Array<Rect>& eventContent);

		void customEventAction(const int32 playerID, const int32 eventCode, const Array<Circle>& eventContent);

		void customEventAction(const int32 playerID, const int32 eventCode, const Grid<Point>& eventContent);

		void customEventAction(const int32 playerID, const int32 eventCode, const Grid<Vec2>& eventContent);

		void customEventAction(const int32 playerID, const int32 eventCode, const Grid<Rect>& eventContent);

		void customEventAction(const int32 playerID, const int32 eventCode, const Grid<Circle>& eventContent);

	private:

		using Scene_t = std::shared_ptr<IScene<State, Data>>;

		using FactoryFunction_t = std::function<Scene_t()>;

		HashTable<State, FactoryFunction_t> m_factories;

		std::shared_ptr<Data> m_data;

		Scene_t m_current;

		Scene_t m_next;

		State m_currentState;

		State m_nextState;

		Optional<State> m_first;

		enum class TransitionState
		{
			None_,

			FadeIn,

			Active,

			FadeOut,

			FadeInOut,

		} m_transitionState = TransitionState::None_;

		Stopwatch m_stopwatch;

		int32 m_transitionTimeMillisec = 1000;

		ColorF m_fadeColor = Palette::Black;

		CrossFade m_crossFade = CrossFade::No;

		bool m_error = false;

		[[nodiscard]]
		bool updateSingle();

		[[nodiscard]]
		bool updateCross();

		[[nodiscard]]
		bool hasError() const noexcept;
	};
}

# include "detail/SivPhotonSceneMaster.ipp"
