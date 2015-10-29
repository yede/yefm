#ifndef YE_CUSTOM_ACTIONS_H
#define YE_CUSTOM_ACTIONS_H

#include <QObject>
#include <QAction>
#include <QHash>
//==============================================================================================================================

class QFileInfo;

struct UsrAction {
	UsrAction(int match, const QStringList &types): match(match), types(types) {}

	int         match;
	QStringList types;
};
//==============================================================================================================================

class CustomActions : public QObject
{
	Q_OBJECT
public:
	explicit CustomActions(QObject *parent = 0);
	~CustomActions();

	static void load(QStringList &result);
	static void save(const QStringList &items);

	void createActions();
	void clear();
	void reset() { m_ready = false; }

	void addActions(QMenu &menu, const QFileInfo &fileInfo);

	bool isReady() const { return m_ready; }

private:
	bool addAction(QMenu &menu, QAction *action);
	bool hasDuplicateAction(QMenu &menu, const QAction *action);

	QAction *createAction(const QString &appName);
	QAction *createAction(const QString &title, const QString &exec, const QString &icon);

signals:

public slots:

public:
	static bool matchDir(int match);
	static bool matchFile(const QStringList &types, const QString &ext);
	static bool matchAny(int match);
	static bool matchAnyFile(int match);

	static bool isDesktopApp(int kind);
	static void insertTypeTag(QString &types, int match);

	static void decodeTypes(QStringList &result, QString &types);
	static int  decodeMatch(QString &types);
	static int  decodeKind(QString &name);

	static bool decode(int &kind, QString &name, QString &exec, QString &icon,
					   int &match, QString &types, const QString &item);
	static void encode(QString &item, int kind, const QString &name, const QString &exec, const QString &icon,
					   int match, const QString &types);

private:
	bool m_ready;
	bool m_dummy_1, m_dummy_2, m_dummy_3, m_dummy_4, m_dummy_5, m_dummy_6, m_dummy_7;
	QHash<QAction*, UsrAction> m_items;
};
//==============================================================================================================================

#endif // YE_CUSTOM_ACTIONS_H
