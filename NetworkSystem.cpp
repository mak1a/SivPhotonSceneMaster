
# pragma once
# define NOMINMAX
# include <LoadBalancing-cpp/inc/Client.h>
# include "NetworkSystem.hpp"

# if SIV3D_PLATFORM(WINDOWS)
# if SIV3D_BUILD(DEBUG)
#   pragma comment (lib, "Common-cpp/lib/Common-cpp_vc16_debug_windows_mt_x64")
#   pragma comment (lib, "Photon-cpp/lib/Photon-cpp_vc16_debug_windows_mt_x64")
#   pragma comment (lib, "LoadBalancing-cpp/lib/LoadBalancing-cpp_vc16_debug_windows_mt_x64")
# else
#   pragma comment (lib, "Common-cpp/lib/Common-cpp_vc16_release_windows_mt_x64")
#   pragma comment (lib, "Photon-cpp/lib/Photon-cpp_vc16_release_windows_mt_x64")
#   pragma comment (lib, "LoadBalancing-cpp/lib/LoadBalancing-cpp_vc16_release_windows_mt_x64")
# endif
# endif


namespace s3d
{
	namespace detail
	{
		[[nodiscard]]
		String ToString(const ExitGames::Common::JString& str)
		{
			return Unicode::FromWstring(std::wstring_view{ str.cstr(), str.length() });
		}

		[[nodiscard]]
		ExitGames::Common::JString ToJString(const StringView s)
		{
			return ExitGames::Common::JString{ Unicode::ToWstring(s).c_str() };
		}
	}

	template <class T, uint8 customTypeIndex>
	class SivCustomType : public ExitGames::Common::CustomType<SivCustomType<T, customTypeIndex>, customTypeIndex>
	{
	public:
	private:

		T m_value;
	public:

		SivCustomType(void) = default;

		SivCustomType(const T& value)
			: ExitGames::Common::CustomType<SivCustomType<T, customTypeIndex>, customTypeIndex>()
			, m_value(value)
		{}

		SivCustomType(const SivCustomType& toCopy)
			: ExitGames::Common::CustomType<SivCustomType<T, customTypeIndex>, customTypeIndex>()
			, m_value(toCopy.m_value)
		{}

		~SivCustomType(void) {}

		SivCustomType& operator=(const SivCustomType& toCopy)
		{
			m_value = toCopy.m_value;
			return *this;
		}

		void cleanup(void)
		{}

		bool compare(const ExitGames::Common::CustomTypeBase& other) const
		{
			return m_value == ((SivCustomType&)other).m_value;
		}

		void duplicate(ExitGames::Common::CustomTypeBase* pRetVal) const
		{
			*reinterpret_cast<SivCustomType*>(pRetVal) = *this;
		}

		void deserialize(const nByte* pData, short length)
		{
			const T* hoge = reinterpret_cast<const T*>(pData);

			std::memcpy(&m_value, hoge, sizeof(T));
		}

		short serialize(nByte* pRetVal) const
		{
			if (pRetVal)
			{
				T* hoge = reinterpret_cast<T*>(pRetVal);
				std::memcpy(hoge, &m_value, sizeof(T));
				pRetVal = reinterpret_cast<nByte*>(hoge);
			}

			return sizeof(T);
		}

		ExitGames::Common::JString& toString(ExitGames::Common::JString& retStr, bool withTypes = false) const
		{
			auto className = Unicode::Widen(typeid(T).name()).split(' ').back();
			//return retStr = detail::ToJString(Format(T{ m_value }));
			return retStr = detail::ToJString(Unicode::Widen(typeid(T).name()));
		}

		T& getValue()
		{
			return m_value;
		}
	};

	using PhotonPoint = SivCustomType<Point, 0>;

	using PhotonVec2 = SivCustomType<Vec2, 1>;

	using PhotonRect = SivCustomType<Rect, 2>;

	using PhotonCircle = SivCustomType<Circle, 3>;
}

namespace s3d
{
	class SivPhoton::SivPhotonDetail : public ExitGames::LoadBalancing::Listener
	{
	public:

		explicit SivPhotonDetail(SivPhoton& context_)
			: m_context{ context_ }
		{
			m_receiveEventFunctions.emplace(0, [this](const int playerID, const nByte eventCode, const ExitGames::Common::Object& eventContent) { receivedCustomType<Point, 0>(playerID, eventCode, eventContent); });
			m_receiveEventFunctions.emplace(1, [this](const int playerID, const nByte eventCode, const ExitGames::Common::Object& eventContent) { receivedCustomType<Vec2, 1>(playerID, eventCode, eventContent); });
			m_receiveEventFunctions.emplace(2, [this](const int playerID, const nByte eventCode, const ExitGames::Common::Object& eventContent) { receivedCustomType<Rect, 2>(playerID, eventCode, eventContent); });
			m_receiveEventFunctions.emplace(3, [this](const int playerID, const nByte eventCode, const ExitGames::Common::Object& eventContent) { receivedCustomType<Circle, 3>(playerID, eventCode, eventContent); });

			m_receiveArrayEventFunctions.emplace(0, [this](const int playerID, const nByte eventCode, const ExitGames::Common::Object* eventContent) { receivedCustomArrayType<Point, 0>(playerID, eventCode, eventContent); });
			m_receiveArrayEventFunctions.emplace(1, [this](const int playerID, const nByte eventCode, const ExitGames::Common::Object* eventContent) { receivedCustomArrayType<Vec2, 1>(playerID, eventCode, eventContent); });
			m_receiveArrayEventFunctions.emplace(2, [this](const int playerID, const nByte eventCode, const ExitGames::Common::Object* eventContent) { receivedCustomArrayType<Rect, 2>(playerID, eventCode, eventContent); });
			m_receiveArrayEventFunctions.emplace(3, [this](const int playerID, const nByte eventCode, const ExitGames::Common::Object* eventContent) { receivedCustomArrayType<Circle, 3>(playerID, eventCode, eventContent); });

			m_receiveGridEventFunctions.emplace(0, [this](const int playerID, const nByte eventCode, const ExitGames::Common::Object* eventContent, const Size size) { receivedCustomGridType<Point, 0>(playerID, eventCode, eventContent, size); });
			m_receiveGridEventFunctions.emplace(1, [this](const int playerID, const nByte eventCode, const ExitGames::Common::Object* eventContent, const Size size) { receivedCustomGridType<Vec2, 1>(playerID, eventCode, eventContent, size); });
			m_receiveGridEventFunctions.emplace(2, [this](const int playerID, const nByte eventCode, const ExitGames::Common::Object* eventContent, const Size size) { receivedCustomGridType<Rect, 2>(playerID, eventCode, eventContent, size); });
			m_receiveGridEventFunctions.emplace(3, [this](const int playerID, const nByte eventCode, const ExitGames::Common::Object* eventContent, const Size size) { receivedCustomGridType<Circle, 3>(playerID, eventCode, eventContent, size); });
		}

		void debugReturn(int debugLevel, const ExitGames::Common::JString& string) override
		{

		}

		void connectionErrorReturn(int errorCode) override
		{
			m_context.connectionErrorReturn(errorCode);
			m_context.m_isUsePhoton = false;
		}

		void clientErrorReturn(int errorCode) override
		{

		}

		void warningReturn(int warningCode) override
		{

		}

		void serverErrorReturn(int errorCode) override
		{

		}

		// 自分でも他人でも、誰かが参加したら呼ばれるコールバック
		void joinRoomEventAction(const int playerID, const ExitGames::Common::JVector<int>& playerIDs, const ExitGames::LoadBalancing::Player& player) override
		{
			Array<int32> ids(playerIDs.getSize());
			for (unsigned i = 0; i < playerIDs.getSize(); ++i)
			{
				ids[i] = playerIDs[i];
			}

			const auto myID = m_context.getClient().getLocalPlayer().getNumber();
			const auto newID = player.getNumber();
			const bool isSelf = (myID == newID);
			m_context.joinRoomEventAction(playerID, ids, isSelf);
		}

		// 他人でも、誰かが退室したら呼ばれるコールバック
		void leaveRoomEventAction(const int playerID, const bool isInactive) override
		{
			m_context.leaveRoomEventAction(playerID, isInactive);
		}

		// ルームで他人が RaiseEvent したら呼ばれるコールバック
		void customEventAction(const int playerID, const nByte eventCode, const ExitGames::Common::Object& eventContent) override
		{
			Print << U"SivPhoton::SivPhotonDetail::customEventAction() [ルームで他人が RaiseEvent したときの処理]";
			Print << U"eventCode: " << int32(eventCode);

			uint8 type = eventContent.getType();

			if (type == ExitGames::Common::TypeCode::CUSTOM)
			{
				const uint8 customType = eventContent.getCustomType();
				m_receiveEventFunctions[customType](playerID, eventCode, eventContent);
				return;
			}

			if (type == ExitGames::Common::TypeCode::HASHTABLE)
			{
				ExitGames::Common::Hashtable eventDataContent = ExitGames::Common::ValueObject<ExitGames::Common::Hashtable>(eventContent).getDataCopy();
				ExitGames::Common::JString arrayType = ExitGames::Common::ValueObject<ExitGames::Common::JString>(eventDataContent.getValue(L"ArrayType")).getDataCopy();
				if (arrayType == L"Array")
				{
					type = eventDataContent.getValue(L"values")->getType();
					if (type == ExitGames::Common::TypeCode::CUSTOM)
					{
						const uint8 customType = eventDataContent.getValue(L"values")->getCustomType();
						m_receiveArrayEventFunctions[customType](playerID, eventCode, eventDataContent.getValue(L"values"));
						return;
					}

					switch (type)
					{
					case ExitGames::Common::TypeCode::INTEGER:
					{
						auto values = ExitGames::Common::ValueObject<int*>(eventDataContent.getValue(L"values")).getDataCopy();
						auto length = *(ExitGames::Common::ValueObject<int*>(eventDataContent.getValue(L"values"))).getSizes();

						Array<int32> data;
						for (const auto i : step(length))
						{
							data << values[i];
						}
						m_context.customEventAction(playerID, eventCode, data);
						return;
					}
					case ExitGames::Common::TypeCode::DOUBLE:
					{
						auto values = ExitGames::Common::ValueObject<double*>(eventDataContent.getValue(L"values")).getDataCopy();
						auto length = *(ExitGames::Common::ValueObject<double*>(eventDataContent.getValue(L"values"))).getSizes();

						Print << length;

						Array<double> data;
						for (const auto i : step(length))
						{
							data << values[i];
						}
						m_context.customEventAction(playerID, eventCode, data);
						return;
					}
					case ExitGames::Common::TypeCode::FLOAT:
					{
						auto values = ExitGames::Common::ValueObject<float*>(eventDataContent.getValue(L"values")).getDataCopy();
						auto length = *(ExitGames::Common::ValueObject<float*>(eventDataContent.getValue(L"values"))).getSizes();

						Array<float> data;
						for (const auto i : step(length))
						{
							data << values[i];
						}
						m_context.customEventAction(playerID, eventCode, data);
						return;
					}
					case ExitGames::Common::TypeCode::BOOLEAN:
					{
						auto values = ExitGames::Common::ValueObject<bool*>(eventDataContent.getValue(L"values")).getDataCopy();
						auto length = *(ExitGames::Common::ValueObject<bool*>(eventDataContent.getValue(L"values"))).getSizes();

						Array<bool> data;
						for (const auto i : step(length))
						{
							data << values[i];
						}
						m_context.customEventAction(playerID, eventCode, data);
						return;
					}
					case ExitGames::Common::TypeCode::STRING:
					{
						auto values = ExitGames::Common::ValueObject<ExitGames::Common::JString*>(eventDataContent.getValue(L"values")).getDataCopy();
						auto length = *(ExitGames::Common::ValueObject<ExitGames::Common::JString*>(eventDataContent.getValue(L"values"))).getSizes();

						Array<String> data;
						for (const auto i : step(length))
						{
							data << detail::ToString(values[i]);
						}
						m_context.customEventAction(playerID, eventCode, data);
						return;
					}
					default:
						break;
					}
				}

				if (arrayType == L"Grid")
				{
					type = eventDataContent.getValue(L"values")->getType();
					Size size = ExitGames::Common::ValueObject<PhotonPoint>(eventDataContent.getValue(L"xy")).getDataCopy().getValue();
					if (type == ExitGames::Common::TypeCode::CUSTOM)
					{
						const uint8 customType = eventDataContent.getValue(L"values")->getCustomType();
						m_receiveGridEventFunctions[customType](playerID, eventCode, eventDataContent.getValue(L"values"), size);
						return;
					}

					switch (type)
					{
					case ExitGames::Common::TypeCode::INTEGER:
					{
						auto values = ExitGames::Common::ValueObject<int*>(eventDataContent.getValue(L"values")).getDataCopy();
						auto length = *(ExitGames::Common::ValueObject<int*>(eventDataContent.getValue(L"values"))).getSizes();

						Array<int32> data;
						for (const auto i : step(length))
						{
							data << values[i];
						}

						Grid<int32> grid(size, data);

						m_context.customEventAction(playerID, eventCode, grid);
						return;
					}
					case ExitGames::Common::TypeCode::DOUBLE:
					{
						auto values = ExitGames::Common::ValueObject<double*>(eventDataContent.getValue(L"values")).getDataCopy();
						auto length = *(ExitGames::Common::ValueObject<double*>(eventDataContent.getValue(L"values"))).getSizes();

						Print << length;

						Array<double> data;
						for (const auto i : step(length))
						{
							data << values[i];
						}

						Grid<double> grid(size, data);

						m_context.customEventAction(playerID, eventCode, grid);
						return;
					}
					case ExitGames::Common::TypeCode::FLOAT:
					{
						auto values = ExitGames::Common::ValueObject<float*>(eventDataContent.getValue(L"values")).getDataCopy();
						auto length = *(ExitGames::Common::ValueObject<float*>(eventDataContent.getValue(L"values"))).getSizes();

						Array<float> data;
						for (const auto i : step(length))
						{
							data << values[i];
						}

						Grid<float> grid(size, data);

						m_context.customEventAction(playerID, eventCode, grid);
						return;
					}
					case ExitGames::Common::TypeCode::BOOLEAN:
					{
						auto values = ExitGames::Common::ValueObject<bool*>(eventDataContent.getValue(L"values")).getDataCopy();
						auto length = *(ExitGames::Common::ValueObject<bool*>(eventDataContent.getValue(L"values"))).getSizes();

						Array<bool> data;
						for (const auto i : step(length))
						{
							data << values[i];
						}

						Grid<bool> grid(size, data);

						m_context.customEventAction(playerID, eventCode, grid);
						return;
					}
					case ExitGames::Common::TypeCode::STRING:
					{
						auto values = ExitGames::Common::ValueObject<ExitGames::Common::JString*>(eventDataContent.getValue(L"values")).getDataCopy();
						auto length = *(ExitGames::Common::ValueObject<ExitGames::Common::JString*>(eventDataContent.getValue(L"values"))).getSizes();

						Array<String> data;
						for (const auto i : step(length))
						{
							data << detail::ToString(values[i]);
						}

						Grid<String> grid(size, data);

						m_context.customEventAction(playerID, eventCode, grid);
						return;
					}
					default:
						break;
					}
				}

				return;
			}

			switch (type)
			{
			case ExitGames::Common::TypeCode::INTEGER:
				m_context.customEventAction(playerID, eventCode, ExitGames::Common::ValueObject<int>(eventContent).getDataCopy());
				return;
			case ExitGames::Common::TypeCode::DOUBLE:
				m_context.customEventAction(playerID, eventCode, ExitGames::Common::ValueObject<double>(eventContent).getDataCopy());
				return;
			case ExitGames::Common::TypeCode::FLOAT:
				m_context.customEventAction(playerID, eventCode, ExitGames::Common::ValueObject<float>(eventContent).getDataCopy());
				return;
			case ExitGames::Common::TypeCode::BOOLEAN:
				m_context.customEventAction(playerID, eventCode, ExitGames::Common::ValueObject<bool>(eventContent).getDataCopy());
				return;
			case ExitGames::Common::TypeCode::STRING:
				m_context.customEventAction(playerID, eventCode, detail::ToString(ExitGames::Common::ValueObject<ExitGames::Common::JString>(eventContent).getDataCopy()));
				return;
			default:
				break;
			}
		}

		// connect() の結果を通知するコールバック
		void connectReturn(int errorCode, const ExitGames::Common::JString& errorString, const ExitGames::Common::JString& region, const ExitGames::Common::JString& cluster) override
		{
			const String errorText = detail::ToString(errorString);
			const String regionText = detail::ToString(region);
			const String clusterText = detail::ToString(cluster);
			m_context.connectReturn(errorCode, errorText, regionText, clusterText);
			if (errorCode)
			{
				m_context.m_isUsePhoton = false;
			}
		}

		// disconnect() の結果を通知するコールバック
		void disconnectReturn() override
		{
			m_context.disconnectReturn();
			m_context.m_isUsePhoton = false;
		}

		// 
		void leaveRoomReturn(int errorCode, const ExitGames::Common::JString& errorString) override
		{
			const String errorText = detail::ToString(errorString);
			m_context.leaveRoomReturn(errorCode, errorText);
		}

		void joinRandomRoomReturn(int localPlayerID, const ExitGames::Common::Hashtable& roomProperties, const ExitGames::Common::Hashtable& playerProperties, int errorCode, const ExitGames::Common::JString& errorString) override
		{
			m_context.joinRandomRoomReturn(localPlayerID, errorCode, detail::ToString(errorString));
		}

		void createRoomReturn(int localPlayerID, const ExitGames::Common::Hashtable& roomProperties, const ExitGames::Common::Hashtable& playerProperties, int errorCode, const ExitGames::Common::JString& errorString) override
		{
			m_context.createRoomReturn(localPlayerID, errorCode, detail::ToString(errorString));
		}

	private:

		SivPhoton& m_context;

		HashTable<uint8, std::function<void(const int, const nByte, const ExitGames::Common::Object&)>> m_receiveEventFunctions;

		HashTable<uint8, std::function<void(const int, const nByte, const ExitGames::Common::Object*)>> m_receiveArrayEventFunctions;

		HashTable<uint8, std::function<void(const int, const nByte, const ExitGames::Common::Object*, const Size)>> m_receiveGridEventFunctions;

		template <class T, uint8 N>
		void receivedCustomType(const int playerID, const nByte eventCode, const ExitGames::Common::Object& eventContent)
		{
			auto value = ExitGames::Common::ValueObject<SivCustomType<T, N>>(eventContent).getDataCopy().getValue();
			m_context.customEventAction(playerID, eventCode, value);
		}

		template <class T, uint8 N>
		void receivedCustomArrayType(const int playerID, const nByte eventCode, const ExitGames::Common::Object* eventContent)
		{
			SivCustomType<T, N>* values = ExitGames::Common::ValueObject<SivCustomType<T, N>*>(eventContent).getDataCopy();
			auto length = *(ExitGames::Common::ValueObject<SivCustomType<T, N>*>(eventContent)).getSizes();

			Array<T> data;
			for (const auto i : step(length))
			{
				data << values[i].getValue();
			}
			m_context.customEventAction(playerID, eventCode, data);
		}

		template <class T, uint8 N>
		void receivedCustomGridType(const int playerID, const nByte eventCode, const ExitGames::Common::Object* eventContent, const Size size)
		{
			SivCustomType<T, N>* values = ExitGames::Common::ValueObject<SivCustomType<T, N>*>(eventContent).getDataCopy();
			auto length = *(ExitGames::Common::ValueObject<SivCustomType<T, N>*>(eventContent)).getSizes();

			Array<T> data;
			for (const auto i : step(length))
			{
				data << values[i].getValue();
			}

			Grid<T> grid{ size, data };
			m_context.customEventAction(playerID, eventCode, grid);
		}
	};
}

namespace s3d
{
	SivPhoton::SivPhoton(const StringView secretPhotonAppID, const StringView photonAppVersion)
		: m_listener{ std::make_unique<SivPhotonDetail>(*this) }
		, m_client{ std::make_unique<ExitGames::LoadBalancing::Client>(*m_listener, detail::ToJString(secretPhotonAppID), detail::ToJString(photonAppVersion)) }
		, m_isUsePhoton{ false }
	{

		PhotonPoint::registerType();
		PhotonVec2::registerType();
		PhotonRect::registerType();
		PhotonCircle::registerType();
	}

	SivPhoton::~SivPhoton()
	{
		Print << U"SivPhoton::~SivPhoton()";

		PhotonPoint::unregisterType();
		PhotonVec2::unregisterType();
		PhotonRect::unregisterType();
		PhotonCircle::unregisterType();

		disconnect();
	}

	void SivPhoton::connect(const StringView userName, const Optional<String>& defaultRoomName)
	{
		Print << U"SivPhoton::connect() [サーバに接続する]";

		m_defaultRoomName = defaultRoomName.value_or(String{ userName });

		const auto userNameJ = detail::ToJString(userName);
		const auto userID = ExitGames::LoadBalancing::AuthenticationValues{}
		.setUserID(userNameJ + GETTIMEMS());

		if (not m_client->connect({ userID, userNameJ }))
		{
			Print << U"ExitGmae::LoadBalancing::Client::connect() failed.";
			return;
		}

		m_client->fetchServerTimestamp();
		m_isUsePhoton = true;
	}

	void SivPhoton::disconnect()
	{
		m_client->disconnect();
	}

	void SivPhoton::update()
	{
		m_client->service();
	}

	void SivPhoton::opJoinRandomRoom(const int32 maxPlayers)
	{
		Print << U"SivPhoton::opJoinRandomRoom(maxPlayers = {}) [既存のランダムなルームに参加する]"_fmt(maxPlayers);

		assert(InRange(maxPlayers, 0, 255));

		m_client->opJoinRandomRoom({}, static_cast<uint8>(Clamp(maxPlayers, 1, 255)));
	}

	void SivPhoton::opJoinRoom(const StringView roomName, const bool rejoin)
	{
		Print << U"SivPhoton::opJoinRoom() [既存の指定したルームに参加する]";

		const auto roomNameJ = detail::ToJString(roomName);

		m_client->opJoinRoom(roomNameJ, rejoin);
	}

	void SivPhoton::opCreateRoom(const StringView roomName, const int32 maxPlayers)
	{
		Print << U"SivPhoton::opCreateRoom() [ルームを新規に作成する]";

		assert(InRange(maxPlayers, 0, 255));

		const auto roomNameJ = detail::ToJString(roomName);
		const auto roomOption = ExitGames::LoadBalancing::RoomOptions()
			.setMaxPlayers(static_cast<uint8>(Clamp(maxPlayers, 1, 255)));

		m_client->opCreateRoom(roomNameJ, roomOption);
	}

	void SivPhoton::opLeaveRoom()
	{
		Print << U"SivPhoton::opLeaveRoom() [ルームを退室する]";

		constexpr bool willComeBack = false;

		m_client->opLeaveRoom(willComeBack);
	}
}

namespace s3d
{
	template<class T>
	void s3d::SivPhoton::opRaiseEvent(uint8 eventCode, const T& value)
	{
		this->opRaiseEvent<T>(eventCode, value);
	}

	template<>
	void SivPhoton::opRaiseEvent(uint8 eventCode, const Rect& value)
	{
		Print << U"opRaiseEvent()";

		constexpr bool reliable = true;
		m_client->opRaiseEvent(reliable, PhotonRect{ value }, eventCode);
	}

	template<>
	void SivPhoton::opRaiseEvent(uint8 eventCode, const Vec2& value)
	{
		Print << U"opRaiseEvent()";

		constexpr bool reliable = true;
		m_client->opRaiseEvent(reliable, PhotonVec2{ value }, eventCode);
	}

	template<>
	void SivPhoton::opRaiseEvent(uint8 eventCode, const Point& value)
	{
		Print << U"opRaiseEvent()";

		constexpr bool reliable = true;
		m_client->opRaiseEvent(reliable, PhotonPoint{ value }, eventCode);
	}

	template<>
	void SivPhoton::opRaiseEvent(uint8 eventCode, const Circle& value)
	{
		Print << U"opRaiseEvent()";

		constexpr bool reliable = true;
		m_client->opRaiseEvent(reliable, PhotonCircle{ value }, eventCode);
	}

	template<>
	void SivPhoton::opRaiseEvent(uint8 eventCode, const Array<Point>& values)
	{
		Print << U"opRaiseEvent()";

		constexpr bool reliable = true;

		Array<PhotonPoint> data;
		for (const auto& v : values)
		{
			data << PhotonPoint{ v };
		}

		ExitGames::Common::Hashtable ev;
		ev.put(L"ArrayType", L"Array");
		ev.put(L"values", data.data(), data.size());

		m_client->opRaiseEvent(reliable, ev, eventCode);
	}

	template<>
	void SivPhoton::opRaiseEvent(uint8 eventCode, const Array<Vec2>& values)
	{
		Print << U"opRaiseEvent()";

		constexpr bool reliable = true;

		Array<PhotonVec2> data;
		for (const auto& v : values)
		{
			data << PhotonVec2{ v };
		}

		ExitGames::Common::Hashtable ev;
		ev.put(L"ArrayType", L"Array");
		ev.put(L"values", data.data(), data.size());

		m_client->opRaiseEvent(reliable, ev, eventCode);
	}

	template<>
	void SivPhoton::opRaiseEvent(uint8 eventCode, const Array<Rect>& values)
	{
		Print << U"opRaiseEvent()";

		constexpr bool reliable = true;

		Array<PhotonRect> data;
		for (const auto& v : values)
		{
			data << PhotonRect{ v };
		}

		ExitGames::Common::Hashtable ev;
		ev.put(L"ArrayType", L"Array");
		ev.put(L"values", data.data(), data.size());

		m_client->opRaiseEvent(reliable, ev, eventCode);
	}

	template<>
	void SivPhoton::opRaiseEvent(uint8 eventCode, const Array<Circle>& values)
	{
		Print << U"opRaiseEvent()";

		constexpr bool reliable = true;

		Array<PhotonCircle> data;
		for (const auto& v : values)
		{
			data << PhotonCircle{ v };
		}

		ExitGames::Common::Hashtable ev;
		ev.put(L"ArrayType", L"Array");
		ev.put(L"values", data.data(), data.size());

		m_client->opRaiseEvent(reliable, ev, eventCode);
	}

	template<>
	void SivPhoton::opRaiseEvent(uint8 eventCode, const Grid<Point>& values)
	{
		Print << U"opRaiseEvent()";

		constexpr bool reliable = true;

		Array<PhotonPoint> data;
		for (const auto& v : values)
		{
			data << PhotonPoint{ v };
		}

		ExitGames::Common::Hashtable ev;
		ev.put(L"ArrayType", L"Grid");
		ev.put(L"xy", PhotonPoint{ Point{values.width(), values.height()} });
		ev.put(L"values", data.data(), data.size());

		m_client->opRaiseEvent(reliable, ev, eventCode);
	}

	template<>
	void SivPhoton::opRaiseEvent(uint8 eventCode, const Grid<Vec2>& values)
	{
		Print << U"opRaiseEvent()";

		constexpr bool reliable = true;

		Array<PhotonVec2> data;
		for (const auto& v : values)
		{
			data << PhotonVec2{ v };
		}

		ExitGames::Common::Hashtable ev;
		ev.put(L"ArrayType", L"Grid");
		ev.put(L"xy", PhotonPoint{ Point{values.width(), values.height()} });
		ev.put(L"values", data.data(), data.size());

		m_client->opRaiseEvent(reliable, ev, eventCode);
	}

	template<>
	void SivPhoton::opRaiseEvent(uint8 eventCode, const Grid<Rect>& values)
	{
		Print << U"opRaiseEvent()";

		constexpr bool reliable = true;

		Array<PhotonRect> data;
		for (const auto& v : values)
		{
			data << PhotonRect{ v };
		}

		ExitGames::Common::Hashtable ev;
		ev.put(L"ArrayType", L"Grid");
		ev.put(L"xy", PhotonPoint{ Point{values.width(), values.height()} });
		ev.put(L"values", data.data(), data.size());

		m_client->opRaiseEvent(reliable, ev, eventCode);
	}

	template<>
	void SivPhoton::opRaiseEvent(uint8 eventCode, const Grid<Circle>& values)
	{
		Print << U"opRaiseEvent()";

		constexpr bool reliable = true;

		Array<PhotonCircle> data;
		for (const auto& v : values)
		{
			data << PhotonCircle{ v };
		}

		ExitGames::Common::Hashtable ev;
		ev.put(L"ArrayType", L"Grid");
		ev.put(L"xy", PhotonPoint{ Point{values.width(), values.height()} });
		ev.put(L"values", data.data(), data.size());

		m_client->opRaiseEvent(reliable, ev, eventCode);
	}

	void SivPhoton::opRaiseEvent(const uint8 eventCode, const int32 value)
	{
		Print << U"opRaiseEvent()";

		constexpr bool reliable = true;
		m_client->opRaiseEvent(reliable, value, eventCode);
	}

	void SivPhoton::opRaiseEvent(const uint8 eventCode, const double value)
	{
		Print << U"opRaiseEvent()";

		constexpr bool reliable = true;
		m_client->opRaiseEvent(reliable, value, eventCode);
	}

	void SivPhoton::opRaiseEvent(const uint8 eventCode, const float value)
	{
		Print << U"opRaiseEvent()";

		constexpr bool reliable = true;
		m_client->opRaiseEvent(reliable, value, eventCode);
	}

	void SivPhoton::opRaiseEvent(const uint8 eventCode, const bool value)
	{
		Print << U"opRaiseEvent()";

		constexpr bool reliable = true;
		m_client->opRaiseEvent(reliable, value, eventCode);
	}

	void SivPhoton::opRaiseEvent(const uint8 eventCode, const StringView value)
	{
		Print << U"opRaiseEvent()";

		constexpr bool reliable = true;
		m_client->opRaiseEvent(reliable, detail::ToJString(value), eventCode);
	}

	void SivPhoton::opRaiseEvent(uint8 eventCode, const Array<int32>& values)
	{
		Print << U"opRaiseEvent()";

		constexpr bool reliable = true;

		ExitGames::Common::Hashtable ev;
		ev.put(L"ArrayType", L"Array");
		ev.put(L"values", values.data(), values.size());

		m_client->opRaiseEvent(reliable, ev, eventCode);
	}

	void SivPhoton::opRaiseEvent(uint8 eventCode, const Array<double>& values)
	{
		Print << U"opRaiseEvent()";

		constexpr bool reliable = true;

		ExitGames::Common::Hashtable ev;
		ev.put(L"ArrayType", L"Array");
		ev.put(L"values", values.data(), values.size());

		m_client->opRaiseEvent(reliable, ev, eventCode);
	}

	void SivPhoton::opRaiseEvent(uint8 eventCode, const Array<float>& values)
	{
		Print << U"opRaiseEvent()";

		constexpr bool reliable = true;

		ExitGames::Common::Hashtable ev;
		ev.put(L"ArrayType", L"Array");
		ev.put(L"values", values.data(), values.size());

		m_client->opRaiseEvent(reliable, ev, eventCode);
	}

	void SivPhoton::opRaiseEvent(uint8 eventCode, const Array<bool>& values)
	{
		Print << U"opRaiseEvent()";

		constexpr bool reliable = true;

		ExitGames::Common::Hashtable ev;
		ev.put(L"ArrayType", L"Array");
		ev.put(L"values", values.data(), values.size());

		m_client->opRaiseEvent(reliable, ev, eventCode);
	}

	void SivPhoton::opRaiseEvent(uint8 eventCode, const Array<String>& values)
	{
		Print << U"opRaiseEvent()";

		constexpr bool reliable = true;

		Array<ExitGames::Common::JString> data;
		for (const auto& v : values)
		{
			data << detail::ToJString(v);
		}

		ExitGames::Common::Hashtable ev;
		ev.put(L"ArrayType", L"Array");
		ev.put(L"values", data.data(), data.size());

		m_client->opRaiseEvent(reliable, ev, eventCode);
	}

	void SivPhoton::opRaiseEvent(uint8 eventCode, const Grid<int32>& values)
	{
		Print << U"opRaiseEvent()";

		constexpr bool reliable = true;

		Array<int32> data = values.asArray();

		ExitGames::Common::Hashtable ev;
		ev.put(L"ArrayType", L"Grid");
		ev.put(L"xy", PhotonPoint{ Point{values.width(), values.height()} });
		ev.put(L"values", data.data(), data.size());

		m_client->opRaiseEvent(reliable, ev, eventCode);
	}

	void SivPhoton::opRaiseEvent(uint8 eventCode, const Grid<double>& values)
	{
		Print << U"opRaiseEvent()";

		constexpr bool reliable = true;

		Array<double> data = values.asArray();

		ExitGames::Common::Hashtable ev;
		ev.put(L"ArrayType", L"Grid");
		ev.put(L"xy", PhotonPoint{ Point{values.width(), values.height()} });
		ev.put(L"values", data.data(), data.size());

		m_client->opRaiseEvent(reliable, ev, eventCode);
	}

	void SivPhoton::opRaiseEvent(uint8 eventCode, const Grid<float>& values)
	{
		Print << U"opRaiseEvent()";

		constexpr bool reliable = true;

		Array<float> data = values.asArray();

		ExitGames::Common::Hashtable ev;
		ev.put(L"ArrayType", L"Grid");
		ev.put(L"xy", PhotonPoint{ Point{values.width(), values.height()} });
		ev.put(L"values", data.data(), data.size());

		m_client->opRaiseEvent(reliable, ev, eventCode);
	}

	void SivPhoton::opRaiseEvent(uint8 eventCode, const Grid<bool>& values)
	{
		Print << U"opRaiseEvent()";

		constexpr bool reliable = true;

		Array<bool> data = values.asArray();

		ExitGames::Common::Hashtable ev;
		ev.put(L"ArrayType", L"Grid");
		ev.put(L"xy", PhotonPoint{ Point{values.width(), values.height()} });
		ev.put(L"values", data.data(), data.size());

		m_client->opRaiseEvent(reliable, ev, eventCode);
	}

	void SivPhoton::opRaiseEvent(uint8 eventCode, const Grid<String>& values)
	{
		Print << U"opRaiseEvent()";

		constexpr bool reliable = true;

		Array<ExitGames::Common::JString> data;
		for (const auto& v : values)
		{
			data << detail::ToJString(v);
		}

		ExitGames::Common::Hashtable ev;
		ev.put(L"ArrayType", L"Grid");
		ev.put(L"xy", PhotonPoint{ Point{values.width(), values.height()} });
		ev.put(L"values", data.data(), data.size());

		m_client->opRaiseEvent(reliable, ev, eventCode);
	}

	String SivPhoton::getName() const
	{
		return detail::ToString(m_client->getLocalPlayer().getName());
	}

	String SivPhoton::getUserID() const
	{
		return detail::ToString(m_client->getLocalPlayer().getUserID());
	}

	Array<String> SivPhoton::getRoomNameList() const
	{
		const auto roomNameList = m_client->getRoomNameList();
		Array<String> result;

		for (uint32 i = 0; i < roomNameList.getSize(); ++i)
		{
			result << detail::ToString(roomNameList[i]);
		}

		return result;
	}

	bool SivPhoton::isInRoom() const
	{
		return m_client->getIsInGameRoom();
	}

	String SivPhoton::getCurrentRoomName() const
	{
		if (not m_client->getIsInGameRoom())
		{
			return{};
		}

		return detail::ToString(m_client->getCurrentlyJoinedRoom().getName());
	}

	int32 SivPhoton::getPlayerCountInCurrentRoom() const
	{
		if (not m_client->getIsInGameRoom())
		{
			return 0;
		}

		return m_client->getCurrentlyJoinedRoom().getPlayerCount();
	}

	int32 SivPhoton::getMaxPlayersInCurrentRoom() const
	{
		if (not m_client->getIsInGameRoom())
		{
			return 0;
		}

		return m_client->getCurrentlyJoinedRoom().getMaxPlayers();
	}

	bool SivPhoton::getIsOpenInCurrentRoom() const
	{
		return m_client->getCurrentlyJoinedRoom().getIsOpen();
	}

	bool SivPhoton::getIsVisibleInCurrentRoom() const
	{
		return m_client->getCurrentlyJoinedRoom().getIsVisible();
	}

	void SivPhoton::setIsOpenInCurrentRoom(const bool isOpen)
	{
		m_client->getCurrentlyJoinedRoom().setIsOpen(isOpen);
	}

	void SivPhoton::setIsVisibleInCurrentRoom(const bool isVisible)
	{
		m_client->getCurrentlyJoinedRoom().setIsVisible(isVisible);
	}

	int32 SivPhoton::getCountGamesRunning() const
	{
		return m_client->getCountGamesRunning();
	}

	int32 SivPhoton::getCountPlayersIngame() const
	{
		return m_client->getCountPlayersIngame();
	}

	int32 SivPhoton::getCountPlayersOnline() const
	{
		return m_client->getCountPlayersOnline();
	}

	Optional<int32> SivPhoton::localPlayerID() const
	{
		const int32 localPlayerID = m_client->getLocalPlayer().getNumber();

		if (localPlayerID < 0)
		{
			return none;
		}

		return localPlayerID;
	}

	bool SivPhoton::isMasterClient() const
	{
		return m_client->getLocalPlayer().getIsMasterClient();
	}

	int32 SivPhoton::getNumber() const
	{
		return m_client->getLocalPlayer().getNumber();
	}

	bool SivPhoton::isUsePhoton() const noexcept
	{
		return m_isUsePhoton;
	}

	void SivPhoton::connectionErrorReturn(const int32 errorCode)
	{
		Print << U"SivPhoton::connectionErrorReturn() [サーバへの接続が失敗したときに呼ばれる]";
		Print << U"errorCode: " << errorCode;
	}

	void SivPhoton::connectReturn(const int32 errorCode, const String& errorString, const String& region, const String& cluster)
	{
		Print << U"SivPhoton::connectReturn()";
		Print << U"error: " << errorString;
		Print << U"region: " << region;
		Print << U"cluster: " << cluster;
	}

	void SivPhoton::disconnectReturn()
	{
		Print << U"SivPhoton::disconnectReturn() [サーバから切断されたときに呼ばれる]";
	}

	void SivPhoton::leaveRoomReturn(const int32 errorCode, const String& errorString)
	{
		Print << U"SivPhoton::leaveRoomReturn() [ルームから退室した結果を処理する]";
		Print << U"- errorCode:" << errorCode;
		Print << U"- errorString:" << errorString;
	}

	void SivPhoton::joinRandomRoomReturn(const int32 localPlayerID, const int32 errorCode, const String& errorString)
	{
		Print << U"SivPhoton::joinRandomRoomReturn()";
		Print << U"localPlayerID:" << localPlayerID;
		Print << U"errorCode:" << errorCode;
		Print << U"errorString:" << errorString;
	}

	void SivPhoton::joinRoomReturn(int32 localPlayerID, int32 errorCode, const String& errorString)
	{
		Print << U"SivPhoton::joinRoomReturn()";
		Print << U"localPlayerID:" << localPlayerID;
		Print << U"errorCode:" << errorCode;
		Print << U"errorString:" << errorString;
	}

	void SivPhoton::joinRoomEventAction(const int32 localPlayerID, const Array<int32>& playerIDs, const bool isSelf)
	{
		Print << U"SivPhoton::joinRoomEventAction() [自分を含め、プレイヤーが参加したら呼ばれる]";
		Print << U"localPlayerID [参加した人の ID]:" << localPlayerID;
		Print << U"playerIDs: [ルームの参加者一覧]" << playerIDs;
		Print << U"isSelf [自分自身の参加？]:" << isSelf;
	}

	void SivPhoton::leaveRoomEventAction(const int32 playerID, const bool isInactive)
	{
		Print << U"SivPhoton::leaveRoomEventAction()";
		Print << U"playerID: " << playerID;
		Print << U"isInactive: " << isInactive;

		if (m_client->getLocalPlayer().getIsMasterClient())
		{
			Print << U"I am now the master client";
		}
		else
		{
			Print << U"I am still not the master client";
		}
	}

	void SivPhoton::createRoomReturn(const int32 localPlayerID, const int32 errorCode, const String& errorString)
	{
		Print << U"SivPhoton::createRoomReturn() [ルームを新規作成した結果を処理する]";
		Print << U"- localPlayerID:" << localPlayerID;
		Print << U"- errorCode:" << errorCode;
		Print << U"- errorString:" << errorString;
	}

	void SivPhoton::customEventAction(const int32 playerID, const int32 eventCode, const int32 eventContent)
	{
		Print << U"SivPhoton::customEventAction(int32)";
		Print << U"playerID: " << playerID;
		Print << U"eventCode: " << eventCode;
		Print << U"eventContent: " << eventContent;
	}

	void SivPhoton::customEventAction(const int32 playerID, const int32 eventCode, const double eventContent)
	{
		Print << U"SivPhoton::customEventAction(double)";
		Print << U"playerID: " << playerID;
		Print << U"eventCode: " << eventCode;
		Print << U"eventContent: " << eventContent;
	}

	void SivPhoton::customEventAction(const int32 playerID, const int32 eventCode, const float eventContent)
	{
		Print << U"SivPhoton::customEventAction(float)";
		Print << U"playerID: " << playerID;
		Print << U"eventCode: " << eventCode;
		Print << U"eventContent: " << eventContent;
	}

	void SivPhoton::customEventAction(const int32 playerID, const int32 eventCode, const bool eventContent)
	{
		Print << U"SivPhoton::customEventAction(bool)";
		Print << U"playerID: " << playerID;
		Print << U"eventCode: " << eventCode;
		Print << U"eventContent: " << eventContent;
	}

	void SivPhoton::customEventAction(const int32 playerID, const int32 eventCode, const String& eventContent)
	{
		Print << U"SivPhoton::customEventAction(String)";
		Print << U"playerID: " << playerID;
		Print << U"eventCode: " << eventCode;
		Print << U"eventContent: " << eventContent;
	}

	void SivPhoton::customEventAction(const int32 playerID, const int32 eventCode, const Array<int32>& eventContent)
	{
		Print << U"SivPhoton::customEventAction(Array<int32>)";
		Print << U"playerID: " << playerID;
		Print << U"eventCode: " << eventCode;
		Print << U"eventContent: " << eventContent;
	}

	void SivPhoton::customEventAction(const int32 playerID, const int32 eventCode, const Array<double>& eventContent)
	{
		Print << U"SivPhoton::customEventAction(Array<double>)";
		Print << U"playerID: " << playerID;
		Print << U"eventCode: " << eventCode;
		Print << U"eventContent: " << eventContent;
	}

	void SivPhoton::customEventAction(const int32 playerID, const int32 eventCode, const Array<float>& eventContent)
	{
		Print << U"SivPhoton::customEventAction(Array<float>)";
		Print << U"playerID: " << playerID;
		Print << U"eventCode: " << eventCode;
		Print << U"eventContent: " << eventContent;
	}

	void SivPhoton::customEventAction(const int32 playerID, const int32 eventCode, const Array<bool>& eventContent)
	{
		Print << U"SivPhoton::customEventAction(Array<bool>)";
		Print << U"playerID: " << playerID;
		Print << U"eventCode: " << eventCode;
		Print << U"eventContent: " << eventContent;
	}

	void SivPhoton::customEventAction(const int32 playerID, const int32 eventCode, const Array<String>& eventContent)
	{
		Print << U"SivPhoton::customEventAction(Array<String>)";
		Print << U"playerID: " << playerID;
		Print << U"eventCode: " << eventCode;
		Print << U"eventContent: " << eventContent;
	}

	void SivPhoton::customEventAction(const int32 playerID, const int32 eventCode, const Grid<int32>& eventContent)
	{
		Print << U"SivPhoton::customEventAction(Grid<int32>)";
		Print << U"playerID: " << playerID;
		Print << U"eventCode: " << eventCode;
		Print << U"eventContent: " << eventContent;
	}

	void SivPhoton::customEventAction(const int32 playerID, const int32 eventCode, const Grid<double>& eventContent)
	{
		Print << U"SivPhoton::customEventAction(Grid<double>)";
		Print << U"playerID: " << playerID;
		Print << U"eventCode: " << eventCode;
		Print << U"eventContent: " << eventContent;
	}

	void SivPhoton::customEventAction(const int32 playerID, const int32 eventCode, const Grid<float>& eventContent)
	{
		Print << U"SivPhoton::customEventAction(Grid<float>)";
		Print << U"playerID: " << playerID;
		Print << U"eventCode: " << eventCode;
		Print << U"eventContent: " << eventContent;
	}

	void SivPhoton::customEventAction(const int32 playerID, const int32 eventCode, const Grid<bool>& eventContent)
	{
		Print << U"SivPhoton::customEventAction(Grid<bool>)";
		Print << U"playerID: " << playerID;
		Print << U"eventCode: " << eventCode;
		Print << U"eventContent: " << eventContent;
	}

	void SivPhoton::customEventAction(const int32 playerID, const int32 eventCode, const Grid<String>& eventContent)
	{
		Print << U"SivPhoton::customEventAction(Grid<String>)";
		Print << U"playerID: " << playerID;
		Print << U"eventCode: " << eventCode;
		Print << U"eventContent: " << eventContent;
	}

	void SivPhoton::customEventAction(const int32 playerID, const int32 eventCode, const Point& eventContent)
	{
		Print << U"SivPhoton::customEventAction(Point)";
		Print << U"playerID: " << playerID;
		Print << U"eventCode: " << eventCode;
		Print << U"eventContent: " << eventContent;
	}

	void SivPhoton::customEventAction(const int32 playerID, const int32 eventCode, const Vec2& eventContent)
	{
		Print << U"SivPhoton::customEventAction(Vec2)";
		Print << U"playerID: " << playerID;
		Print << U"eventCode: " << eventCode;
		Print << U"eventContent: " << eventContent;
	}

	void SivPhoton::customEventAction(const int32 playerID, const int32 eventCode, const Rect& eventContent)
	{
		Print << U"SivPhoton::customEventAction(Rect)";
		Print << U"playerID: " << playerID;
		Print << U"eventCode: " << eventCode;
		Print << U"eventContent: " << eventContent;
	}

	void SivPhoton::customEventAction(const int32 playerID, const int32 eventCode, const Circle& eventContent)
	{
		Print << U"SivPhoton::customEventAction(Circle)";
		Print << U"playerID: " << playerID;
		Print << U"eventCode: " << eventCode;
		Print << U"eventContent: " << eventContent;
	}

	void SivPhoton::customEventAction(const int32 playerID, const int32 eventCode, const Array<Point>& eventContent)
	{
		Print << U"SivPhoton::customEventAction(Array<Point>)";
		Print << U"playerID: " << playerID;
		Print << U"eventCode: " << eventCode;
		Print << U"eventContent: " << eventContent;
	}

	void SivPhoton::customEventAction(const int32 playerID, const int32 eventCode, const Array<Vec2>& eventContent)
	{
		Print << U"SivPhoton::customEventAction(Array<Vec2>)";
		Print << U"playerID: " << playerID;
		Print << U"eventCode: " << eventCode;
		Print << U"eventContent: " << eventContent;
	}

	void SivPhoton::customEventAction(const int32 playerID, const int32 eventCode, const Array<Rect>& eventContent)
	{
		Print << U"SivPhoton::customEventAction(Array<Rect>)";
		Print << U"playerID: " << playerID;
		Print << U"eventCode: " << eventCode;
		Print << U"eventContent: " << eventContent;
	}

	void SivPhoton::customEventAction(const int32 playerID, const int32 eventCode, const Array<Circle>& eventContent)
	{
		Print << U"SivPhoton::customEventAction(Array<Circle>)";
		Print << U"playerID: " << playerID;
		Print << U"eventCode: " << eventCode;
		Print << U"eventContent: " << eventContent;
	}

	void SivPhoton::customEventAction(const int32 playerID, const int32 eventCode, const Grid<Point>& eventContent)
	{
		Print << U"SivPhoton::customEventAction(Grid<Point>)";
		Print << U"playerID: " << playerID;
		Print << U"eventCode: " << eventCode;
		Print << U"eventContent: " << eventContent;
	}

	void SivPhoton::customEventAction(const int32 playerID, const int32 eventCode, const Grid<Vec2>& eventContent)
	{
		Print << U"SivPhoton::customEventAction(Grid<Vec2>)";
		Print << U"playerID: " << playerID;
		Print << U"eventCode: " << eventCode;
		Print << U"eventContent: " << eventContent;
	}

	void SivPhoton::customEventAction(const int32 playerID, const int32 eventCode, const Grid<Rect>& eventContent)
	{
		Print << U"SivPhoton::customEventAction(Grid<Rect>)";
		Print << U"playerID: " << playerID;
		Print << U"eventCode: " << eventCode;
		Print << U"eventContent: " << eventContent;
	}

	void SivPhoton::customEventAction(const int32 playerID, const int32 eventCode, const Grid<Circle>& eventContent)
	{
		Print << U"SivPhoton::customEventAction(Array<Circle>)";
		Print << U"playerID: " << playerID;
		Print << U"eventCode: " << eventCode;
		Print << U"eventContent: " << eventContent;
	}

	ExitGames::LoadBalancing::Client& SivPhoton::getClient()
	{
		assert(m_client);

		return *m_client;
	}


}
