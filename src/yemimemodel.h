#ifndef MIMEMODEL_H
#define MIMEMODEL_H


class MimeModel : public QObject
{
	Q_OBJECT
public:
	explicit MimeModel(QObject *parent = 0);
	~MimeModel();

signals:

public slots:
};

#endif // MIMEMODEL_H
