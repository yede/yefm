#ifndef YE_MOUNT_H
#define YE_MOUNT_H

#include <QObject>
//==================================================================================

struct MountItem {
	const char *path;
	const char *type;
};

class MountItems
{
public:
	MountItems();
	~MountItems();

	void append(char *path, char *type);
	void clear() { m_count = 0; }
	void setValid(bool valid) { m_valid = valid; }

	const MountItem &at(int i) const { return m_items[i]; }
	int count() const { return m_count; }
	bool isValid() const { return m_valid; }

private:
	MountItem *m_items;
	int        m_count;
	int        m_quota;
	bool       m_valid;
};
//==================================================================================

class Mtab
{
public:
	Mtab();
	~Mtab();

	const MountItems &items() const { return m_items; }

	bool readNewItems(bool restarted);

private:
	bool allocMemory(int size);
	bool backup();
	bool openMtab();
	bool parseLine(char *line);
	void parseItems();

private:
	char      *m_backup;
	int        m_bakLen;
	char      *m_memory;
	int        m_memLen;
	MountItems m_items;
};
//==================================================================================

class Mount : public QObject
{
    Q_OBJECT
public:
    explicit Mount(QObject *parent = 0);
	~Mount();

	void restart();

signals:
	void afterMounts(const MountItems &mounts);

public slots:
	void readMounts();

private:
	bool m_stopWorking;
	bool m_restarted;
	Mtab m_mounts;
};

#endif // YE_MOUNT_H
