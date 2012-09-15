#pragma once

// [Specification of class]
// OWNER class must has static member function
//   => static OWNER * restoreOwner(int) 
//
// PROTOCOL class mst has member function 
//   => int ownerKey(void)
//   => int code(void)

#define DEFAULT_MAX_PENDING_SIZE	1024
#define DEFAULT_MAX_WORKING_SIZE	10240

template <class OWNER, class PROTOCOL>
class ProtocolManagerImpl {
public:
	ProtocolManagerImpl() 
		: maxPendingSizePerCode_(DEFAULT_MAX_PENDING_SIZE)
		, maxWorkingSizePerCode_(DEFAULT_MAX_WORKING_SIZE) 
	{
	}

	~ProtocolManagerImpl() {}
	
	typedef boost::function< void (OWNER *, PROTOCOL) > FIRE_FUNCTOR;
	typedef std::list<PROTOCOL> listProtocol_t;
	typedef std::set<PROTOCOL> setProtocol_t;
	typedef std::map<int, setProtocol_t> mapOwnerKey_t;

	typedef typename listProtocol_t::iterator ListProtocolIterator; 
	typedef typename setProtocol_t::iterator SetProtocolIterator; 
	typedef typename mapOwnerKey_t::iterator MapOwnerKeyIterator; 
	typedef typename mapOwnerKey_t::value_type MapOwnerKeyValueType;
	
	struct protocolStoage_t {
		listProtocol_t protList;
		mapOwnerKey_t ownerKeyMap;

		void insert(PROTOCOL prot) {
			protList.push_back(prot);

			MapOwnerKeyIterator it = ownerKeyMap.find(prot->ownerKey());
			if(it != ownerKeyMap.end()) {
				it->second.insert(prot);
			} else {
				setProtocol_t ps;
				ps.insert(prot);
				ownerKeyMap.insert(MapOwnerKeyValueType(prot->ownerKey(), ps));
			}
		}

		void erase(PROTOCOL prot) {
			MapOwnerKeyIterator it = ownerKeyMap.find(prot->ownerKey());
			if(it != ownerKeyMap.end()) {
				SetProtocolIterator itSet = it->second.find(prot);
				if(it->second.end() != itSet)
					it->second.erase(itSet);
			}
			protList.remove(prot);
		}

		void erase(int ownerKey) {
			MapOwnerKeyIterator it = ownerKeyMap.find(ownerKey);
			if(it != ownerKeyMap.end()) {
				if(it->second.size() > 0) {
					SetProtocolIterator itSet = it->second.begin();
					for( ; itSet != it->second.end(); itSet++) {
						protList.remove(*itSet);
					}
					it->second.clear();
				}
			}
		}
	
		size_t size() {
#define DEBUG
#ifdef DEBUG
			size_t tot = 0;
			MapOwnerKeyIterator it = ownerKeyMap.begin();
			for( ; it != ownerKeyMap.end(); it++) {
				tot += it->second.size();
			}
			assert(protList.size() == tot && "protocolStoage_t::size is not same");
#endif
			return protList.size();
		}
	
		PROTOCOL pop_front() {
			ListProtocolIterator it = protList.begin();
			if(it == protList.end())
				return PROTOCOL();

			PROTOCOL ret = *it;
			protList.pop_front();

			MapOwnerKeyIterator itMap = ownerKeyMap.find(ret->ownerKey());
			if(itMap != ownerKeyMap.end()) {
				SetProtocolIterator itSet = itMap->second.find(ret);
				if(itSet != itMap->second.end()) {
					itMap->second.erase(itSet);
				}
			}
			return ret;
		}
	};

	// -----------------------------------------------------
	// [Data Structure]
	// <code> - storage -> list : <protocol>
	//                  -> map  : <ownerKey> - <protocol>
	// -----------------------------------------------------
	typedef std::map<boost::int32_t, struct protocolStoage_t> mapStorage_t;
	typedef typename mapStorage_t::iterator MapStorageIterator;
	typedef typename mapStorage_t::value_type MapStorageValueType;

public:
	void clear() {
		ScopedMutexLock(lock_);
		pendingProtocols_.clear();
		workingProtocols_.clear();
	}

	void deleteAllProtocolOf(int ownerKey) {
		ScopedMutexLock(lock_);
		LOG_DEBUG("# ProtocolManager::deleteAllProtocolOf() : key = %d, count = %d/%d [START]\n"
			, ownerKey, pendingProtocolCountAll(), workingProtocolCountAll());

		MapStorageIterator itP = pendingProtocols_.begin();	
		for( ; itP != pendingProtocols_.end(); itP++) {
			struct protocolStoage_t &ps = itP->second;
			ps.erase(ownerKey);
		}

		MapStorageIterator itW = workingProtocols_.begin();	
		for( ; itW != workingProtocols_.end(); itW++) {
			struct protocolStoage_t &ps = itW->second;
			ps.erase(ownerKey);
		}

		LOG_DEBUG("# ProtocolManager::deleteAllProtocolOf() : key = %d, count = %d/%d [END]\n"
			, ownerKey, pendingProtocolCountAll(), workingProtocolCountAll());
	}

	void triggerAllPendingProtocol() {
		ScopedMutexLock(lock_);
		listProtocol_t tempList;
		MapStorageIterator itP = pendingProtocols_.begin();	
		for( ; itP != pendingProtocols_.end(); itP++) {
			struct protocolStoage_t &ps = itP->second;
			while(ps.size() > 0) {
				PROTOCOL p = ps.pop_front();
				tempList.push_back(p);
			}
		}

		// fire!
		while(tempList.size() > 0) {
			ListProtocolIterator it = tempList.begin();
			PROTOCOL p = *it;
			tempList.pop_front();
			if(p) fireFunctor_(OWNER::restoreOwner(p->ownerKey()), p);	
		}
		LOG_DEBUG("# ProtocolManager::triggerAllPendingProtocol() : pending count = %d\n", pendingProtocolCountAll());
	}

	void triggerPendingProtocol(boost::int32_t code) {
		ScopedMutexLock(lock_);
		struct protocolStoage_t &ps = _findPendingProtocolSet(code);
		if(ps.size() <= 0) {
			return;
		}

		PROTOCOL p = ps.pop_front();
		if(p) fireFunctor_(OWNER::restoreOwner(p->ownerKey()), p);	
		LOG_DEBUG("# ProtocolManager::triggerPendingProtocol() : code = 0x%x, pending count = %d\n" 
			, code, pendingProtocolCount(code));
	}

	void addPendingProtocol(PROTOCOL prot) {
		ScopedMutexLock(lock_);
		if(!prot) return;
		_removeWorkingProtocol(prot);

		if(pendingProtocolCount(prot->code()) > maxPendingSizePerCode_) {
			LOG_TRACE("# ProtocolManager::addPendingProtocol() : max size limit error : %d/%d\n"
				, pendingProtocolCount(prot->code()), maxPendingSizePerCode_);
			return;	
		}

		struct protocolStoage_t &ps = _findPendingProtocolSet(prot->code());
		ps.insert(prot);
		LOG_TRACE("# ProtocolManager::addPendingProtocol() : pending count = %d\n", pendingProtocolCount(prot->code()));
	}

	void addWorkingProtocol(PROTOCOL prot) {
		ScopedMutexLock(lock_);
		if(!prot) return;
		_removePendingProtocol(prot);

		if(pendingProtocolCount(prot->code()) > maxWorkingSizePerCode_) {
			LOG_TRACE("# ProtocolManager::addWorkingProtocol() : max size limit error : %d/%d\n"
				, workingProtocolCount(prot->code()), maxWorkingSizePerCode_);
			return;	
		}

		struct protocolStoage_t &ps = _findWorkingProtocolSet(prot->code());
		ps.insert(prot);
		LOG_TRACE("# ProtocolManager::addWorkingProtocol() : working count = %d\n", workingProtocolCount(prot->code()));
	}

	void completeWorkProtocol(PROTOCOL prot) {
		ScopedMutexLock(lock_);
		if(!prot) return;

		boost::int32_t code = prot->code();

		LOG_TRACE("# ProtocolManager::completeWorkProtocol() : count(p/w) = %d/%d\n"
			, pendingProtocolCount(code), workingProtocolCount(code));

		_removeWorkingProtocol(prot);

		triggerPendingProtocol(code);

	}

	size_t pendingProtocolCountAll() {
		ScopedMutexLock(lock_);
		size_t tot = 0;
		MapStorageIterator it = pendingProtocols_.begin();	
		for( ; it != pendingProtocols_.end(); it++) {
			tot += it->second.size();
		}
		return tot;
	}

	size_t workingProtocolCountAll() {
		ScopedMutexLock(lock_);
		size_t tot = 0;
		MapStorageIterator it = workingProtocols_.begin();	
		for( ; it != workingProtocols_.end(); it++) {
			tot += it->second.size();
		}
		return tot;
	}

	size_t pendingProtocolCount(boost::int32_t code) {
		ScopedMutexLock(lock_);
		struct protocolStoage_t &ps = _findPendingProtocolSet(code);
		return ps.size();
	}

	size_t workingProtocolCount(boost::int32_t code) {
		ScopedMutexLock(lock_);
		struct protocolStoage_t &ps = _findWorkingProtocolSet(code);
		return ps.size();
	}

	void setFireFunctor(FIRE_FUNCTOR functor) {
		fireFunctor_ = functor;
	}

private:
	void _removePendingProtocol(PROTOCOL prot) {
		struct protocolStoage_t &ps = _findPendingProtocolSet(prot->code());
		ps.erase(prot);
	}

	void _removeWorkingProtocol(PROTOCOL prot) {
		struct protocolStoage_t &ps = _findWorkingProtocolSet(prot->code());
		ps.erase(prot);
	}

	struct protocolStoage_t & _findPendingProtocolSet(boost::int32_t code) {
		MapStorageIterator it = pendingProtocols_.find(code);
		if(it != pendingProtocols_.end()) {
			return it->second;
		} else {
			// new protocol set!
			struct protocolStoage_t newStorage;
			pendingProtocols_.insert(MapStorageValueType(code, newStorage));
			return _findPendingProtocolSet(code);
		}

		assert(false && "never reach here!");
	}

	struct protocolStoage_t & _findWorkingProtocolSet(boost::int32_t code) {
		MapStorageIterator it = workingProtocols_.find(code);
		if(it != workingProtocols_.end()) {
			return it->second;
		} else {
			// new protocol set!
			struct protocolStoage_t newStorage;
			workingProtocols_.insert(MapStorageValueType(code, newStorage));
			return _findWorkingProtocolSet(code);
		}

		assert(false && "never reach here!");
	}

private:
	mapStorage_t pendingProtocols_;
	mapStorage_t workingProtocols_;
	FIRE_FUNCTOR fireFunctor_;
	size_t maxPendingSizePerCode_;
	size_t maxWorkingSizePerCode_;
	Mutex lock_;
};

