#ifndef _GROUP_H
#define _GROUP_H

#include <qobject.h>
#include <qvaluelist.h>

class QString;
class QDomDocument;
class QDomNode;

/**
 * class Group: 
 *   This class abstracts the concept of transfer group by means of which
 *   the user can sort his transfers into categories.
 *   Moreover this class calculates informations such as:
 *   -> the size obtained by the sum of all the transfer's size
 *   -> the size obtained by the sum of all the transfer's processed size
 *   -> the global progress percentage within the group
 *   -> the global speed within the group
 *   This is useful becouse saves each view from calculating this data and
 *   allows each one to get this info simply calling the getInfo() method.
 */
class Group
{
    public:
    
    struct Info
    {
        QString name;
        
        unsigned long totalSize;
        unsigned long processedSize;
        int percent;
        int speed;
    };
    

    public:
    Group(const QString& name);
    Group(QDomNode * n);
            
    bool read(QDomNode * n);
    void write(QDomNode * n) const;
    
    const Info& getInfo() const {return info;}
    void setName(const QString& name);

    void about() const;
    
    private:
    Info info;
};

class GroupList : public QObject, public QValueList<Group *>
{
Q_OBJECT

    public:
    
    typedef QValueListIterator<Group *> iterator;
    typedef QValueListConstIterator<Group *> constIterator;

    GroupList();
    GroupList(const GroupList&);
    GroupList(Group group);

    Group * getGroup(const QString& groupName) const;
    void addGroup(Group group);
    void addGroups(const GroupList& list);
    void delGroup(Group group);
    void delGroups(const GroupList& list);
    void modifyGroup(const QString& groupName, Group * group);
        
    bool read(QDomDocument * doc);
    void write(QDomDocument * doc) const;

    void about() const;
};

#endif
