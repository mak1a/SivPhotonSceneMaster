# include "SivPhotonSceneMaster.hpp"

namespace s3d::NetworkSystem
{
	template <class State, class Data>
	inline IScene<State, Data>::InitData::InitData(const State_t& _state, const std::shared_ptr<Data_t>& data, SivPhotonSceneMaster<State_t, Data_t>* manager)
		: state{ _state }
		, _s{ data }
		, _m{ manager } {}

	template <class State, class Data>
	inline IScene<State, Data>::IScene(const InitData& init)
		: m_state{ init.state }
		, m_data{ init._s }
		, m_manager{ init._m } {}

	template <class State, class Data>
	inline void IScene<State, Data>::drawFadeIn(const double t) const
	{
		draw();

		Transformer2D transform{ Mat3x2::Identity(), Transformer2D::Target::SetLocal };

		Scene::Rect().draw(ColorF{ m_manager->getFadeColor(), (1.0 - t) });
	}

	template <class State, class Data>
	inline void IScene<State, Data>::drawFadeOut(const double t) const
	{
		draw();

		Transformer2D transform{ Mat3x2::Identity(), Transformer2D::Target::SetLocal };

		Scene::Rect().draw(ColorF{ m_manager->getFadeColor(), t });
	}

	template <class State, class Data>
	inline const typename IScene<State, Data>::State_t& IScene<State, Data>::getState() const
	{
		return m_state;
	}

	template <class State, class Data>
	template <class DataType, std::enable_if_t<not std::disjunction_v<std::is_array<DataType>, std::is_void<DataType>>>*>
	inline DataType& IScene<State, Data>::getData() const
	{
		return *m_data;
	}

	template <class State, class Data>
	inline bool IScene<State, Data>::changeScene(const State_t& state, const Duration& transitionTime, const CrossFade crossFade)
	{
		return changeScene(state, static_cast<int32>(transitionTime.count() * 1000), crossFade);
	}

	template <class State, class Data>
	inline bool IScene<State, Data>::changeScene(const State_t& state, const int32 transitionTimeMillisec, const CrossFade crossFade)
	{
		return m_manager->changeScene(state, transitionTimeMillisec, crossFade);
	}

	template <class State, class Data>
	inline void IScene<State, Data>::notifyError()
	{
		return m_manager->notifyError();
	}


	template <class State, class Data>
	inline SivPhotonSceneMaster<State, Data>::SivPhotonSceneMaster(StringView secretPhotonAppID, StringView photonAppVersion)
		: SivPhoton(secretPhotonAppID, photonAppVersion)
	{
		if constexpr (not std::is_void_v<Data>)
		{
			m_data = std::make_shared<Data>();
		}
	}

	template <class State, class Data>
	inline SivPhotonSceneMaster<State, Data>::SivPhotonSceneMaster(const std::shared_ptr<Data>& data, StringView secretPhotonAppID, StringView photonAppVersion)
		: m_data{ data }
		, SivPhoton(secretPhotonAppID, photonAppVersion){}

	template <class State, class Data>
	template <class SceneType>
	inline SivPhotonSceneMaster<State, Data>& SivPhotonSceneMaster<State, Data>::add(const State& state)
	{
		typename SceneType::InitData initData{ state, m_data, this };

		auto factory = [=]() {
			return std::make_shared<SceneType>(initData);
		};

		auto it = m_factories.find(state);

		if (it != m_factories.end())
		{
			it->second = factory;
		}
		else
		{
			m_factories.emplace(state, factory);

			if (not m_first)
			{
				m_first = state;
			}
		}

		return *this;
	}

	template <class State, class Data>
	inline bool SivPhotonSceneMaster<State, Data>::init(const State& state)
	{
		if (m_current)
		{
			return false;
		}

		auto it = m_factories.find(state);

		if (it == m_factories.end())
		{
			return false;
		}

		m_currentState = state;

		m_current = it->second();

		if (hasError())
		{
			return false;
		}

		m_transitionState = TransitionState::FadeIn;

		m_stopwatch.restart();

		return true;
	}

	template <class State, class Data>
	inline bool SivPhotonSceneMaster<State, Data>::updateScene()
	{
		if (hasError())
		{
			return false;
		}

		if (not m_current)
		{
			if (not m_first)
			{
				return true;
			}
			else if (not init(*m_first))
			{
				return false;
			}
		}

		if (m_crossFade)
		{
			return updateCross();
		}
		else
		{
			return updateSingle();
		}
	}

	template <class State, class Data>
	inline void SivPhotonSceneMaster<State, Data>::drawScene() const
	{
		if (not m_current)
		{
			return;
		}

		if ((m_transitionState == TransitionState::Active)
			|| (m_transitionTimeMillisec <= 0))
		{
			m_current->draw();
		}

		const double elapsed = m_stopwatch.msF();
		const double t = (m_transitionTimeMillisec ? (elapsed / m_transitionTimeMillisec) : 1.0);

		if (m_transitionState == TransitionState::FadeIn)
		{
			m_current->drawFadeIn(t);
		}
		else if (m_transitionState == TransitionState::FadeOut)
		{
			m_current->drawFadeOut(t);
		}
		else if (m_transitionState == TransitionState::FadeInOut)
		{
			m_current->drawFadeOut(t);

			if (m_next)
			{
				m_next->drawFadeIn(t);
			}
		}
	}

	template <class State, class Data>
	inline bool SivPhotonSceneMaster<State, Data>::update()
	{
		if (not updateScene())
		{
			return false;
		}

		drawScene();

		return true;
	}

	template <class State, class Data>
	inline std::shared_ptr<Data> SivPhotonSceneMaster<State, Data>::get() noexcept
	{
		return m_data;
	}

	template <class State, class Data>
	inline const std::shared_ptr<const Data> SivPhotonSceneMaster<State, Data>::get() const noexcept
	{
		return m_data;
	}

	template <class State, class Data>
	inline bool SivPhotonSceneMaster<State, Data>::changeScene(const State& state, const Duration& transitionTime, const CrossFade crossFade)
	{
		return changeScene(state, static_cast<int32>(transitionTime.count() * 1000), crossFade);
	}

	template <class State, class Data>
	inline bool SivPhotonSceneMaster<State, Data>::changeScene(const State& state, const int32 transitionTimeMillisec, CrossFade crossFade)
	{
		if (state == m_currentState)
		{
			crossFade = CrossFade::No;
		}

		if (not m_factories.contains(state))
		{
			return false;
		}

		m_nextState = state;

		m_crossFade = crossFade;

		if (crossFade)
		{
			m_transitionTimeMillisec = transitionTimeMillisec;

			m_transitionState = TransitionState::FadeInOut;

			m_next = m_factories[m_nextState]();

			if (hasError())
			{
				return false;
			}

			m_currentState = m_nextState;

			m_stopwatch.restart();
		}
		else
		{
			m_transitionTimeMillisec = (transitionTimeMillisec / 2);

			m_transitionState = TransitionState::FadeOut;

			m_stopwatch.restart();
		}

		return true;
	}

	template <class State, class Data>
	inline SivPhotonSceneMaster<State, Data>& SivPhotonSceneMaster<State, Data>::setFadeColor(const ColorF& color) noexcept
	{
		m_fadeColor = color;

		return *this;
	}

	template <class State, class Data>
	inline const ColorF& SivPhotonSceneMaster<State, Data>::getFadeColor() const noexcept
	{
		return m_fadeColor;
	}

	template <class State, class Data>
	inline void SivPhotonSceneMaster<State, Data>::notifyError() noexcept
	{
		m_error = true;
	}

	template <class State, class Data>
	inline bool SivPhotonSceneMaster<State, Data>::updateSingle()
	{
		double elapsed = m_stopwatch.msF();

		if ((m_transitionState == TransitionState::FadeOut)
			&& (m_transitionTimeMillisec <= elapsed))
		{
			m_current = nullptr;

			m_current = m_factories[m_nextState]();

			if (hasError())
			{
				return false;
			}

			m_currentState = m_nextState;

			m_transitionState = TransitionState::FadeIn;

			m_stopwatch.restart();

			elapsed = 0.0;
		}

		if ((m_transitionState == TransitionState::FadeIn)
			&& (m_transitionTimeMillisec <= elapsed))
		{
			m_stopwatch.reset();

			m_transitionState = TransitionState::Active;
		}

		const double t = (m_transitionTimeMillisec ? (elapsed / m_transitionTimeMillisec) : 1.0);

		switch (m_transitionState)
		{
		case TransitionState::FadeIn:
			m_current->updateFadeIn(t);
			break;
		case TransitionState::Active:
			m_current->update();
			if (this->isUsePhoton())
			{
				SivPhoton::update();
			}
			break;
		case TransitionState::FadeOut:
			m_current->updateFadeOut(t);
			break;
		default:
			return false;
		}

		return (not hasError());
	}

	template <class State, class Data>
	inline bool SivPhotonSceneMaster<State, Data>::updateCross()
	{
		const double elapsed = m_stopwatch.msF();

		if ((m_transitionState == TransitionState::FadeInOut)
			&& (m_transitionTimeMillisec <= elapsed))
		{
			m_current = m_next;

			m_next = nullptr;

			m_stopwatch.reset();

			m_transitionState = TransitionState::Active;
		}

		if (m_transitionState == TransitionState::Active)
		{
			m_current->update();
		}
		else
		{
			assert(m_transitionTimeMillisec);

			const double t = (m_transitionTimeMillisec ? (elapsed / m_transitionTimeMillisec) : 1.0);

			m_current->updateFadeOut(t);

			if (hasError())
			{
				return false;
			}

			m_next->updateFadeIn(t);
		}

		return (not hasError());
	}

	template <class State, class Data>
	inline bool SivPhotonSceneMaster<State, Data>::hasError() const noexcept
	{
		return m_error;
	}


	// 通信関係のあれこれ
	template<class State, class Data>
	inline void IScene<State, Data>::connect(const StringView userName, const Optional<String>& defaultRoomName)
	{
		m_manager->connect(userName, defaultRoomName);
	}

	template<class State, class Data>
	inline void IScene<State, Data>::disconnect()
	{
		m_manager->disconnect();
	}

	template<class State, class Data>
	inline void IScene<State, Data>::opJoinRandomRoom(const int32 maxPlayers)
	{
		m_manager->opJoinRandomRoom(maxPlayers);
	}

	template<class State, class Data>
	inline void IScene<State, Data>::opJoinRoom(const StringView roomName, const bool rejoin)
	{
		m_manager->opJoinRoom(roomName, rejoin);
	}

	template<class State, class Data>
	inline void IScene<State, Data>::opCreateRoom(const StringView roomName, const int32 maxPlayers)
	{
		m_manager->opCreateRoom(roomName, maxPlayers);
	}

	template<class State, class Data>
	inline void IScene<State, Data>::opLeaveRoom()
	{
		m_manager->opLeaveRoom();
	}

	template<class State, class Data>
	template<class T>
	inline void IScene<State, Data>::opRaiseEvent(uint8 eventCode, const T& value)
	{
		m_manager->opRaiseEvent(eventCode, value);
	}

	template<class State, class Data>
	inline void IScene<State, Data>::opRaiseEvent(const uint8 eventCode, const int32 value)
	{
		m_manager->opRaiseEvent(eventCode, value);
	}

	template<class State, class Data>
	inline void IScene<State, Data>::opRaiseEvent(const uint8 eventCode, const double value)
	{
		m_manager->opRaiseEvent(eventCode, value);
	}

	template<class State, class Data>
	inline void IScene<State, Data>::opRaiseEvent(const uint8 eventCode, const float value)
	{
		m_manager->opRaiseEvent(eventCode, value);
	}

	template<class State, class Data>
	inline void IScene<State, Data>::opRaiseEvent(const uint8 eventCode, const bool value)
	{
		m_manager->opRaiseEvent(eventCode, value);
	}

	template<class State, class Data>
	inline void IScene<State, Data>::opRaiseEvent(const uint8 eventCode, const StringView value)
	{
		m_manager->opRaiseEvent(eventCode, value);
	}

	template<class State, class Data>
	inline void IScene<State, Data>::opRaiseEvent(uint8 eventCode, const Array<int32>& values)
	{
		m_manager->opRaiseEvent(eventCode, values);
	}

	template<class State, class Data>
	inline void IScene<State, Data>::opRaiseEvent(uint8 eventCode, const Array<double>& values)
	{
		m_manager->opRaiseEvent(eventCode, values);
	}

	template<class State, class Data>
	inline void IScene<State, Data>::opRaiseEvent(uint8 eventCode, const Array<float>& values)
	{
		m_manager->opRaiseEvent(eventCode, values);
	}

	template<class State, class Data>
	inline void IScene<State, Data>::opRaiseEvent(uint8 eventCode, const Array<bool>& values)
	{
		m_manager->opRaiseEvent(eventCode, values);
	}

	template<class State, class Data>
	inline void IScene<State, Data>::opRaiseEvent(uint8 eventCode, const Array<String>& values)
	{
		m_manager->opRaiseEvent(eventCode, values);
	}

	template<class State, class Data>
	inline void IScene<State, Data>::opRaiseEvent(uint8 eventCode, const Grid<int32>& values)
	{
		m_manager->opRaiseEvent(eventCode, values);
	}

	template<class State, class Data>
	inline void IScene<State, Data>::opRaiseEvent(uint8 eventCode, const Grid<double>& values)
	{
		m_manager->opRaiseEvent(eventCode, values);
	}

	template<class State, class Data>
	inline void IScene<State, Data>::opRaiseEvent(uint8 eventCode, const Grid<float>& values)
	{
		m_manager->opRaiseEvent(eventCode, values);
	}

	template<class State, class Data>
	inline void IScene<State, Data>::opRaiseEvent(uint8 eventCode, const Grid<bool>& values)
	{
		m_manager->opRaiseEvent(eventCode, values);
	}

	template<class State, class Data>
	inline void IScene<State, Data>::opRaiseEvent(uint8 eventCode, const Grid<String>& values)
	{
		m_manager->opRaiseEvent(eventCode, values);
	}

	template<class State, class Data>
	inline String IScene<State, Data>::getName() const
	{
		return m_manager->getName();
	}

	template<class State, class Data>
	inline String IScene<State, Data>::getUserID() const
	{
		return m_manager->getUserID();
	}

	template<class State, class Data>
	inline Array<String> IScene<State, Data>::getRoomNameList() const
	{
		return m_manager->getRoomNameList();
	}

	template<class State, class Data>
	inline bool IScene<State, Data>::isInRoom() const
	{
		return m_manager->isInRoom();
	}

	template<class State, class Data>
	inline String IScene<State, Data>::getCurrentRoomName() const
	{
		return m_manager->getCurrentRoomName();
	}

	template<class State, class Data>
	inline inline int32 IScene<State, Data>::getPlayerCountInCurrentRoom() const
	{
		return m_manager->getPlayerCountInCurrentRoom();
	}

	template<class State, class Data>
	inline inline int32 IScene<State, Data>::getMaxPlayersInCurrentRoom() const
	{
		return m_manager->getMaxPlayersInCurrentRoom();
	}

	template<class State, class Data>
	inline bool IScene<State, Data>::getIsOpenInCurrentRoom() const
	{
		return m_manager->getIsOpenInCurrentRoom();
	}

	template<class State, class Data>
	inline bool IScene<State, Data>::getIsVisibleInCurrentRoom() const
	{
		return m_manager->getIsVisibleInCurrentRoom();
	}

	template<class State, class Data>
	inline void IScene<State, Data>::setIsOpenInCurrentRoom(const bool isOpen)
	{
		m_manager->setIsOpenInCurrentRoom(isOpen);
	}

	template<class State, class Data>
	inline void IScene<State, Data>::setIsVisibleInCurrentRoom(const bool isVisible)
	{
		m_manager->setIsVisibleInCurrentRoom(isVisible);
	}

	template<class State, class Data>
	inline int32 IScene<State, Data>::getCountGamesRunning() const
	{
		return m_manager->getCountGamesRunning();
	}

	template<class State, class Data>
	inline int32 IScene<State, Data>::getCountPlayersIngame() const
	{
		return m_manager->getCountPlayersIngame();
	}

	template<class State, class Data>
	inline int32 IScene<State, Data>::getCountPlayersOnline() const
	{
		return m_manager->getCountPlayersOnline();
	}

	template<class State, class Data>
	inline Optional<int32> IScene<State, Data>::localPlayerID() const
	{
		return m_manager->localPlayerID();
	}

	template<class State, class Data>
	inline bool IScene<State, Data>::isMasterClient() const
	{
		return m_manager->isMasterClient();
	}

	template<class State, class Data>
	inline int32 IScene<State, Data>::getNumber() const
	{
		return m_manager->getNumber();
	}

	template<class State, class Data>
	inline void IScene<State, Data>::connectionErrorReturn(const int32 errorCode)
	{
		Print << U"IScene<State, Data>::connectionErrorReturn() [サーバへの接続が失敗したときに呼ばれる]";
		Print << U"errorCode: " << errorCode;
	}

	template<class State, class Data>
	inline void IScene<State, Data>::connectReturn(const int32 errorCode, const String& errorString, const String& region, const String& cluster)
	{
		Print << U"IScene<State, Data>::connectReturn()";
		Print << U"error: " << errorString;
		Print << U"region: " << region;
		Print << U"cluster: " << cluster;
	}

	template<class State, class Data>
	inline void IScene<State, Data>::disconnectReturn()
	{
		Print << U"IScene<State, Data>::disconnectReturn() [サーバから切断されたときに呼ばれる]";
	}

	template<class State, class Data>
	inline void IScene<State, Data>::leaveRoomReturn(const int32 errorCode, const String& errorString)
	{
		Print << U"IScene<State, Data>::leaveRoomReturn() [ルームから退室した結果を処理する]";
		Print << U"- errorCode:" << errorCode;
		Print << U"- errorString:" << errorString;
	}

	template<class State, class Data>
	inline void IScene<State, Data>::joinRandomRoomReturn(const int32 localPlayerID, const int32 errorCode, const String& errorString)
	{
		Print << U"IScene<State, Data>::joinRandomRoomReturn()";
		Print << U"localPlayerID:" << localPlayerID;
		Print << U"errorCode:" << errorCode;
		Print << U"errorString:" << errorString;
	}

	template<class State, class Data>
	inline void IScene<State, Data>::joinRoomReturn(int32 localPlayerID, int32 errorCode, const String& errorString)
	{
		Print << U"IScene<State, Data>::joinRoomReturn()";
		Print << U"localPlayerID:" << localPlayerID;
		Print << U"errorCode:" << errorCode;
		Print << U"errorString:" << errorString;
	}

	template<class State, class Data>
	inline void IScene<State, Data>::joinRoomEventAction(const int32 localPlayerID, const Array<int32>& playerIDs, const bool isSelf)
	{
		Print << U"IScene<State, Data>::joinRoomEventAction() [自分を含め、プレイヤーが参加したら呼ばれる]";
		Print << U"localPlayerID [参加した人の ID]:" << localPlayerID;
		Print << U"playerIDs: [ルームの参加者一覧]" << playerIDs;
		Print << U"isSelf [自分自身の参加？]:" << isSelf;
	}

	template<class State, class Data>
	inline void IScene<State, Data>::leaveRoomEventAction(const int32 playerID, const bool isInactive)
	{
		Print << U"IScene<State, Data>::leaveRoomEventAction()";
		Print << U"playerID: " << playerID;
		Print << U"isInactive: " << isInactive;

		if (m_manager->isMasterClient())
		{
			Print << U"I am now the master client";
		}
		else
		{
			Print << U"I am still not the master client";
		}
	}

	template<class State, class Data>
	inline void IScene<State, Data>::createRoomReturn(const int32 localPlayerID, const int32 errorCode, const String& errorString)
	{
		Print << U"IScene<State, Data>::createRoomReturn() [ルームを新規作成した結果を処理する]";
		Print << U"- localPlayerID:" << localPlayerID;
		Print << U"- errorCode:" << errorCode;
		Print << U"- errorString:" << errorString;
	}

	template<class State, class Data>
	inline void IScene<State, Data>::customEventAction(const int32 playerID, const int32 eventCode, const int32 eventContent)
	{
		Print << U"IScene<State, Data>::customEventAction(int32)";
		Print << U"playerID: " << playerID;
		Print << U"eventCode: " << eventCode;
		Print << U"eventContent: " << eventContent;
	}

	template<class State, class Data>
	inline void IScene<State, Data>::customEventAction(const int32 playerID, const int32 eventCode, const double eventContent)
	{
		Print << U"IScene<State, Data>::customEventAction(double)";
		Print << U"playerID: " << playerID;
		Print << U"eventCode: " << eventCode;
		Print << U"eventContent: " << eventContent;
	}

	template<class State, class Data>
	inline void IScene<State, Data>::customEventAction(const int32 playerID, const int32 eventCode, const float eventContent)
	{
		Print << U"IScene<State, Data>::customEventAction(float)";
		Print << U"playerID: " << playerID;
		Print << U"eventCode: " << eventCode;
		Print << U"eventContent: " << eventContent;
	}

	template<class State, class Data>
	inline void IScene<State, Data>::customEventAction(const int32 playerID, const int32 eventCode, const bool eventContent)
	{
		Print << U"IScene<State, Data>::customEventAction(bool)";
		Print << U"playerID: " << playerID;
		Print << U"eventCode: " << eventCode;
		Print << U"eventContent: " << eventContent;
	}

	template<class State, class Data>
	inline void IScene<State, Data>::customEventAction(const int32 playerID, const int32 eventCode, const String& eventContent)
	{
		Print << U"IScene<State, Data>::customEventAction(String)";
		Print << U"playerID: " << playerID;
		Print << U"eventCode: " << eventCode;
		Print << U"eventContent: " << eventContent;
	}

	template<class State, class Data>
	inline void IScene<State, Data>::customEventAction(const int32 playerID, const int32 eventCode, const Array<int32>& eventContent)
	{
		Print << U"IScene<State, Data>::customEventAction(Array<int32>)";
		Print << U"playerID: " << playerID;
		Print << U"eventCode: " << eventCode;
		Print << U"eventContent: " << eventContent;
	}

	template<class State, class Data>
	inline void IScene<State, Data>::customEventAction(const int32 playerID, const int32 eventCode, const Array<double>& eventContent)
	{
		Print << U"IScene<State, Data>::customEventAction(Array<double>)";
		Print << U"playerID: " << playerID;
		Print << U"eventCode: " << eventCode;
		Print << U"eventContent: " << eventContent;
	}

	template<class State, class Data>
	inline void IScene<State, Data>::customEventAction(const int32 playerID, const int32 eventCode, const Array<float>& eventContent)
	{
		Print << U"IScene<State, Data>::customEventAction(Array<float>)";
		Print << U"playerID: " << playerID;
		Print << U"eventCode: " << eventCode;
		Print << U"eventContent: " << eventContent;
	}

	template<class State, class Data>
	inline void IScene<State, Data>::customEventAction(const int32 playerID, const int32 eventCode, const Array<bool>& eventContent)
	{
		Print << U"IScene<State, Data>::customEventAction(Array<bool>)";
		Print << U"playerID: " << playerID;
		Print << U"eventCode: " << eventCode;
		Print << U"eventContent: " << eventContent;
	}

	template<class State, class Data>
	inline void IScene<State, Data>::customEventAction(const int32 playerID, const int32 eventCode, const Array<String>& eventContent)
	{
		Print << U"IScene<State, Data>::customEventAction(Array<String>)";
		Print << U"playerID: " << playerID;
		Print << U"eventCode: " << eventCode;
		Print << U"eventContent: " << eventContent;
	}

	template<class State, class Data>
	inline void IScene<State, Data>::customEventAction(const int32 playerID, const int32 eventCode, const Grid<int32>& eventContent)
	{
		Print << U"IScene<State, Data>::customEventAction(Grid<int32>)";
		Print << U"playerID: " << playerID;
		Print << U"eventCode: " << eventCode;
		Print << U"eventContent: " << eventContent;
	}

	template<class State, class Data>
	inline void IScene<State, Data>::customEventAction(const int32 playerID, const int32 eventCode, const Grid<double>& eventContent)
	{
		Print << U"IScene<State, Data>::customEventAction(Grid<double>)";
		Print << U"playerID: " << playerID;
		Print << U"eventCode: " << eventCode;
		Print << U"eventContent: " << eventContent;
	}

	template<class State, class Data>
	inline void IScene<State, Data>::customEventAction(const int32 playerID, const int32 eventCode, const Grid<float>& eventContent)
	{
		Print << U"IScene<State, Data>::customEventAction(Grid<float>)";
		Print << U"playerID: " << playerID;
		Print << U"eventCode: " << eventCode;
		Print << U"eventContent: " << eventContent;
	}

	template<class State, class Data>
	inline void IScene<State, Data>::customEventAction(const int32 playerID, const int32 eventCode, const Grid<bool>& eventContent)
	{
		Print << U"IScene<State, Data>::customEventAction(Grid<bool>)";
		Print << U"playerID: " << playerID;
		Print << U"eventCode: " << eventCode;
		Print << U"eventContent: " << eventContent;
	}

	template<class State, class Data>
	inline void IScene<State, Data>::customEventAction(const int32 playerID, const int32 eventCode, const Grid<String>& eventContent)
	{
		Print << U"IScene<State, Data>::customEventAction(Grid<String>)";
		Print << U"playerID: " << playerID;
		Print << U"eventCode: " << eventCode;
		Print << U"eventContent: " << eventContent;
	}

	template<class State, class Data>
	inline void IScene<State, Data>::customEventAction(const int32 playerID, const int32 eventCode, const Point& eventContent)
	{
		Print << U"IScene<State, Data>::customEventAction(Point)";
		Print << U"playerID: " << playerID;
		Print << U"eventCode: " << eventCode;
		Print << U"eventContent: " << eventContent;
	}

	template<class State, class Data>
	inline void IScene<State, Data>::customEventAction(const int32 playerID, const int32 eventCode, const Vec2& eventContent)
	{
		Print << U"IScene<State, Data>::customEventAction(Vec2)";
		Print << U"playerID: " << playerID;
		Print << U"eventCode: " << eventCode;
		Print << U"eventContent: " << eventContent;
	}

	template<class State, class Data>
	inline void IScene<State, Data>::customEventAction(const int32 playerID, const int32 eventCode, const Rect& eventContent)
	{
		Print << U"IScene<State, Data>::customEventAction(Rect)";
		Print << U"playerID: " << playerID;
		Print << U"eventCode: " << eventCode;
		Print << U"eventContent: " << eventContent;
	}

	template<class State, class Data>
	inline void IScene<State, Data>::customEventAction(const int32 playerID, const int32 eventCode, const Circle& eventContent)
	{
		Print << U"IScene<State, Data>::customEventAction(Circle)";
		Print << U"playerID: " << playerID;
		Print << U"eventCode: " << eventCode;
		Print << U"eventContent: " << eventContent;
	}

	template<class State, class Data>
	inline void IScene<State, Data>::customEventAction(const int32 playerID, const int32 eventCode, const Array<Point>& eventContent)
	{
		Print << U"IScene<State, Data>::customEventAction(Array<Point>)";
		Print << U"playerID: " << playerID;
		Print << U"eventCode: " << eventCode;
		Print << U"eventContent: " << eventContent;
	}

	template<class State, class Data>
	inline void IScene<State, Data>::customEventAction(const int32 playerID, const int32 eventCode, const Array<Vec2>& eventContent)
	{
		Print << U"IScene<State, Data>::customEventAction(Array<Vec2>)";
		Print << U"playerID: " << playerID;
		Print << U"eventCode: " << eventCode;
		Print << U"eventContent: " << eventContent;
	}

	template<class State, class Data>
	inline void IScene<State, Data>::customEventAction(const int32 playerID, const int32 eventCode, const Array<Rect>& eventContent)
	{
		Print << U"IScene<State, Data>::customEventAction(Array<Rect>)";
		Print << U"playerID: " << playerID;
		Print << U"eventCode: " << eventCode;
		Print << U"eventContent: " << eventContent;
	}

	template<class State, class Data>
	inline void IScene<State, Data>::customEventAction(const int32 playerID, const int32 eventCode, const Array<Circle>& eventContent)
	{
		Print << U"IScene<State, Data>::customEventAction(Array<Circle>)";
		Print << U"playerID: " << playerID;
		Print << U"eventCode: " << eventCode;
		Print << U"eventContent: " << eventContent;
	}

	template<class State, class Data>
	inline void IScene<State, Data>::customEventAction(const int32 playerID, const int32 eventCode, const Grid<Point>& eventContent)
	{
		Print << U"IScene<State, Data>::customEventAction(Grid<Point>)";
		Print << U"playerID: " << playerID;
		Print << U"eventCode: " << eventCode;
		Print << U"eventContent: " << eventContent;
	}

	template<class State, class Data>
	inline void IScene<State, Data>::customEventAction(const int32 playerID, const int32 eventCode, const Grid<Vec2>& eventContent)
	{
		Print << U"IScene<State, Data>::customEventAction(Grid<Vec2>)";
		Print << U"playerID: " << playerID;
		Print << U"eventCode: " << eventCode;
		Print << U"eventContent: " << eventContent;
	}

	template<class State, class Data>
	inline void IScene<State, Data>::customEventAction(const int32 playerID, const int32 eventCode, const Grid<Rect>& eventContent)
	{
		Print << U"IScene<State, Data>::customEventAction(Grid<Rect>)";
		Print << U"playerID: " << playerID;
		Print << U"eventCode: " << eventCode;
		Print << U"eventContent: " << eventContent;
	}

	template<class State, class Data>
	inline void IScene<State, Data>::customEventAction(const int32 playerID, const int32 eventCode, const Grid<Circle>& eventContent)
	{
		Print << U"IScene<State, Data>::customEventAction(Array<Circle>)";
		Print << U"playerID: " << playerID;
		Print << U"eventCode: " << eventCode;
		Print << U"eventContent: " << eventContent;
	}


	template<class State, class Data>
	inline void SivPhotonSceneMaster<State, Data>::connectionErrorReturn(const int32 errorCode)
	{
		m_current->connectionErrorReturn(errorCode);
	}

	template<class State, class Data>
	inline void SivPhotonSceneMaster<State, Data>::connectReturn(const int32 errorCode, const String& errorString, const String& region, const String& cluster)
	{
		m_current->connectReturn(errorCode, errorString, region, cluster);
	}

	template<class State, class Data>
	inline void SivPhotonSceneMaster<State, Data>::disconnectReturn()
	{
		m_current->disconnectReturn();
	}

	template<class State, class Data>
	inline void SivPhotonSceneMaster<State, Data>::leaveRoomReturn(const int32 errorCode, const String& errorString)
	{
		m_current->leaveRoomReturn(errorCode, errorString);
	}

	template<class State, class Data>
	inline void SivPhotonSceneMaster<State, Data>::joinRandomRoomReturn(const int32 localPlayerID, const int32 errorCode, const String& errorString)
	{
		m_current->joinRandomRoomReturn(localPlayerID, errorCode, errorString);
	}

	template<class State, class Data>
	inline void SivPhotonSceneMaster<State, Data>::joinRoomReturn(int32 localPlayerID, int32 errorCode, const String& errorString)
	{
		m_current->joinRoomReturn(localPlayerID, errorCode, errorString);
	}

	template<class State, class Data>
	inline void SivPhotonSceneMaster<State, Data>::joinRoomEventAction(const int32 localPlayerID, const Array<int32>& playerIDs, const bool isSelf)
	{
		m_current->joinRoomEventAction(localPlayerID, playerIDs, isSelf);
	}

	template<class State, class Data>
	inline void SivPhotonSceneMaster<State, Data>::leaveRoomEventAction(const int32 playerID, const bool isInactive)
	{
		m_current->leaveRoomEventAction(playerID, isInactive);
	}

	template<class State, class Data>
	inline void SivPhotonSceneMaster<State, Data>::createRoomReturn(const int32 localPlayerID, const int32 errorCode, const String& errorString)
	{
		m_current->createRoomReturn(localPlayerID, errorCode, errorString);
	}

	template<class State, class Data>
	inline void SivPhotonSceneMaster<State, Data>::customEventAction(const int32 playerID, const int32 eventCode, const int32 eventContent)
	{
		m_current->customEventAction(playerID, eventCode, eventContent);
	}

	template<class State, class Data>
	inline void SivPhotonSceneMaster<State, Data>::customEventAction(const int32 playerID, const int32 eventCode, const double eventContent)
	{
		m_current->customEventAction(playerID, eventCode, eventContent);
	}

	template<class State, class Data>
	inline void SivPhotonSceneMaster<State, Data>::customEventAction(const int32 playerID, const int32 eventCode, const float eventContent)
	{
		m_current->customEventAction(playerID, eventCode, eventContent);
	}

	template<class State, class Data>
	inline void SivPhotonSceneMaster<State, Data>::customEventAction(const int32 playerID, const int32 eventCode, const bool eventContent)
	{
		m_current->customEventAction(playerID, eventCode, eventContent);
	}

	template<class State, class Data>
	inline void SivPhotonSceneMaster<State, Data>::customEventAction(const int32 playerID, const int32 eventCode, const String& eventContent)
	{
		m_current->customEventAction(playerID, eventCode, eventContent);
	}

	template<class State, class Data>
	inline void SivPhotonSceneMaster<State, Data>::customEventAction(const int32 playerID, const int32 eventCode, const Array<int32>& eventContent)
	{
		m_current->customEventAction(playerID, eventCode, eventContent);
	}

	template<class State, class Data>
	inline void SivPhotonSceneMaster<State, Data>::customEventAction(const int32 playerID, const int32 eventCode, const Array<double>& eventContent)
	{
		m_current->customEventAction(playerID, eventCode, eventContent);
	}

	template<class State, class Data>
	inline void SivPhotonSceneMaster<State, Data>::customEventAction(const int32 playerID, const int32 eventCode, const Array<float>& eventContent)
	{
		m_current->customEventAction(playerID, eventCode, eventContent);
	}

	template<class State, class Data>
	inline void SivPhotonSceneMaster<State, Data>::customEventAction(const int32 playerID, const int32 eventCode, const Array<bool>& eventContent)
	{
		m_current->customEventAction(playerID, eventCode, eventContent);
	}

	template<class State, class Data>
	inline void SivPhotonSceneMaster<State, Data>::customEventAction(const int32 playerID, const int32 eventCode, const Array<String>& eventContent)
	{
		m_current->customEventAction(playerID, eventCode, eventContent);
	}

	template<class State, class Data>
	inline void SivPhotonSceneMaster<State, Data>::customEventAction(const int32 playerID, const int32 eventCode, const Grid<int32>& eventContent)
	{
		m_current->customEventAction(playerID, eventCode, eventContent);
	}

	template<class State, class Data>
	inline void SivPhotonSceneMaster<State, Data>::customEventAction(const int32 playerID, const int32 eventCode, const Grid<double>& eventContent)
	{
		m_current->customEventAction(playerID, eventCode, eventContent);
	}

	template<class State, class Data>
	inline void SivPhotonSceneMaster<State, Data>::customEventAction(const int32 playerID, const int32 eventCode, const Grid<float>& eventContent)
	{
		m_current->customEventAction(playerID, eventCode, eventContent);
	}

	template<class State, class Data>
	inline void SivPhotonSceneMaster<State, Data>::customEventAction(const int32 playerID, const int32 eventCode, const Grid<bool>& eventContent)
	{
		m_current->customEventAction(playerID, eventCode, eventContent);
	}

	template<class State, class Data>
	inline void SivPhotonSceneMaster<State, Data>::customEventAction(const int32 playerID, const int32 eventCode, const Grid<String>& eventContent)
	{
		m_current->customEventAction(playerID, eventCode, eventContent);
	}

	template<class State, class Data>
	inline void SivPhotonSceneMaster<State, Data>::customEventAction(const int32 playerID, const int32 eventCode, const Point& eventContent)
	{
		m_current->customEventAction(playerID, eventCode, eventContent);
	}

	template<class State, class Data>
	inline void SivPhotonSceneMaster<State, Data>::customEventAction(const int32 playerID, const int32 eventCode, const Vec2& eventContent)
	{
		m_current->customEventAction(playerID, eventCode, eventContent);
	}

	template<class State, class Data>
	inline void SivPhotonSceneMaster<State, Data>::customEventAction(const int32 playerID, const int32 eventCode, const Rect& eventContent)
	{
		m_current->customEventAction(playerID, eventCode, eventContent);
	}

	template<class State, class Data>
	inline void SivPhotonSceneMaster<State, Data>::customEventAction(const int32 playerID, const int32 eventCode, const Circle& eventContent)
	{
		m_current->customEventAction(playerID, eventCode, eventContent);
	}

	template<class State, class Data>
	inline void SivPhotonSceneMaster<State, Data>::customEventAction(const int32 playerID, const int32 eventCode, const Array<Point>& eventContent)
	{
		m_current->customEventAction(playerID, eventCode, eventContent);
	}

	template<class State, class Data>
	inline void SivPhotonSceneMaster<State, Data>::customEventAction(const int32 playerID, const int32 eventCode, const Array<Vec2>& eventContent)
	{
		m_current->customEventAction(playerID, eventCode, eventContent);
	}

	template<class State, class Data>
	inline void SivPhotonSceneMaster<State, Data>::customEventAction(const int32 playerID, const int32 eventCode, const Array<Rect>& eventContent)
	{
		m_current->customEventAction(playerID, eventCode, eventContent);
	}

	template<class State, class Data>
	inline void SivPhotonSceneMaster<State, Data>::customEventAction(const int32 playerID, const int32 eventCode, const Array<Circle>& eventContent)
	{
		m_current->customEventAction(playerID, eventCode, eventContent);
	}

	template<class State, class Data>
	inline void SivPhotonSceneMaster<State, Data>::customEventAction(const int32 playerID, const int32 eventCode, const Grid<Point>& eventContent)
	{
		m_current->customEventAction(playerID, eventCode, eventContent);
	}

	template<class State, class Data>
	inline void SivPhotonSceneMaster<State, Data>::customEventAction(const int32 playerID, const int32 eventCode, const Grid<Vec2>& eventContent)
	{
		m_current->customEventAction(playerID, eventCode, eventContent);
	}

	template<class State, class Data>
	inline void SivPhotonSceneMaster<State, Data>::customEventAction(const int32 playerID, const int32 eventCode, const Grid<Rect>& eventContent)
	{
		m_current->customEventAction(playerID, eventCode, eventContent);
	}

	template<class State, class Data>
	inline void SivPhotonSceneMaster<State, Data>::customEventAction(const int32 playerID, const int32 eventCode, const Grid<Circle>& eventContent)
	{
		m_current->customEventAction(playerID, eventCode, eventContent);
	}
}
