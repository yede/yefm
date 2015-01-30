#include <QTimer>
#include <QFile>
#include <QDebug>
#include "yemount.h"
//==================================================================================

MountItems::MountItems()
	: m_items(0), m_count(0), m_quota(0), m_valid(false)
{
}

MountItems::~MountItems()
{
	if (m_items) free(m_items);
}

void MountItems::append(char *path, char *type)
{
	if (m_count >= m_quota) {
		int cnt = m_quota + 32;
		MountItem *mem = (MountItem *) realloc(m_items, sizeof(MountItem) * cnt);
		if (!mem) return;
		m_items = mem;
		m_quota = cnt;
	}
	m_items[m_count].path = path;
	m_items[m_count].type = type;
	m_count++;
}

//==================================================================================
//
//==================================================================================
//#define MTAB "/etc/mtab"
#define MTAB "/proc/self/mounts"

Mtab::Mtab()
	: m_backup(0)
	, m_bakLen(0)
	, m_memory(0)
	, m_memLen(0)
{
}

Mtab::~Mtab()
{
	if (m_backup) { free(m_backup); }
	if (m_memory) { free(m_memory); }
}
//==================================================================================

bool Mtab::allocMemory(int size)
{
	char *mem = (char *) realloc(m_memory, size);
	if (!mem) return false;

	m_memory = mem;
	m_memLen = size;
	return true;
}

bool Mtab::backup()
{
	if (m_bakLen < m_memLen) {
		char *mem = (char *) realloc(m_backup, m_memLen);
		if (!mem) return false;
		m_backup = mem;
		m_bakLen = m_memLen;
	}
	memcpy(m_backup, m_memory, m_memLen);
	return true;
}
//==================================================================================

#define LEN_INIT 2048
#define LEN_STEP 1024

bool Mtab::openMtab()
{
	QFile file(MTAB);
	if (!file.open(QFile::ReadOnly)) return false;

	bool ok = false;
	int len = LEN_INIT;
	while (true) {
		if (len > m_memLen && !allocMemory(len)) break;
		memset(m_memory, 0, m_memLen);
		int sz = file.read(m_memory, m_memLen);
	//	qDebug("Mtab::openMtab(): sz=%d, len=%d, m_memLen=%d", sz, len, m_memLen);
		if (sz < 0) break;
		if (sz < m_memLen) { ok = true; break; }
		len = m_memLen + LEN_STEP;
		file.reset();
	}
	file.close();
	return ok;
}

bool Mtab::parseLine(char *line)
{
	char *p = line;
	int i = 0;

	while (*p != ' ' && *p != '\0') { p++; i++; }
	if (*p == '\0' || i == 0) return false;

	p++;
	char *path = p;
	i = 0;

	while (*p != ' ' && *p != '\0') { p++; i++; }
	if (*p == '\0' || i == 0) return false;

	*p = '\0';
	p++;
	char *type = p;
	i = 0;

	while (*p != ' ' && *p != '\0') { p++; i++; }
	if (*p == '\0' || i == 0) return false;

	*p = '\0';
	if (*line == '/' &&
//		memcmp(type, "rootfs", i)   != 0 &&
//		memcmp(type, "proc", i)     != 0 &&
		memcmp(type, "devtmpfs", i) != 0 &&
//		memcmp(type, "devpts", i)   != 0 &&
		memcmp(type, "sysfs", i)    != 0 &&
		memcmp(type, "tmpfs", i)    != 0)
	{
		m_items.append(path, type);
	}

	return true;
}

void Mtab::parseItems()
{
	m_items.clear();
	char *line = m_memory;
	char *p = m_memory;
	while (true) {
		if (*p == '\n') *p = '\0';
		if (*p == '\0') { if (!parseLine(line)) break; line = p; line++; }
		p++;
	}
}

bool Mtab::readNewItems(bool restarted)
{
	m_items.setValid(false);
	if (!openMtab()) return false;

	if (restarted ||
		m_bakLen != m_memLen ||
		memcmp(m_memory, m_backup, m_memLen) != 0)
	{
		backup();
		parseItems();
		m_items.setValid(true);
		return true;
	}

	return false;
}

//==================================================================================
//
//==================================================================================

Mount::Mount(QObject *parent)
	: QObject(parent)
	, m_stopWorking(false)
	, m_restarted(false)
{
}

Mount::~Mount()
{
	m_stopWorking = true;
}
//==================================================================================

void Mount::restart()
{
	m_restarted = true;
}

void Mount::readMounts()
{
	if (m_stopWorking) return;

	if (m_mounts.readNewItems(m_restarted)) {
		if (m_restarted) m_restarted = false;
		emit afterMounts(m_mounts.items());
	}
}
//==================================================================================
