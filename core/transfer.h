/* This file is part of the KDE project

   Copyright (C) 2004 Dario Massarin <nekkar@libero.it>
   Copyright (C) 2008 - 2011 Lukas Appelhans <l.appelhans@gmx.de>

   This program is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.
*/

#ifndef TRANSFER_H
#define TRANSFER_H

#include "job.h"
#include "kget_export.h"

#include <QElapsedTimer>
#include <QUrl>

#include <KIO/Global>

class QDomElement;

class Signature;
class TransferHandler;
class TransferFactory;
class TransferGroup;
class Scheduler;
class TransferTreeModel;
class FileModel;
class Verifier;

class KGET_EXPORT Transfer : public Job
{
    Q_OBJECT
    friend class TransferHandler;
    friend class TransferTreeModel;

public:
    /**
     * Here we define the flags that should be shared by all the transfers.
     * A transfer should also be able to define additional flags, in the future.
     */
    enum TransferChange {
        Tc_None = 0x00000000,
        // These flags respect the Model columns order NOTE: The model only checks the last 8 bits, so all values which need to be updated by the model should
        // look like: 0x000000xx
        Tc_Source = 0x00000001,
        Tc_FileName = 0x00000002,
        Tc_Status = 0x00000004,
        Tc_TotalSize = 0x00000008,
        Tc_Percent = 0x00000010,
        Tc_DownloadSpeed = 0x00000020,
        Tc_RemainingTime = 0x00000040,
        // Misc
        Tc_UploadSpeed = 0x00000100,
        Tc_UploadLimit = 0x00000200,
        Tc_DownloadLimit = 0x00000400,
        Tc_CanResume = 0x00000800,
        Tc_DownloadedSize = 0x00001000,
        Tc_UploadedSize = 0x00002000,
        Tc_Log = 0x00004000,
        Tc_Group = 0x00008000,
        Tc_Selection = 0x00010000,
    };

    enum Capability {
        Cap_SpeedLimit = 0x00000001,
        Cap_MultipleMirrors = 0x00000002,
        Cap_Resuming = 0x00000004,
        Cap_Renaming = 0x00000008,
        Cap_Moving = 0x00000010,
        Cap_FindFilesize = 0x00000020,
    };
    Q_DECLARE_FLAGS(Capabilities, Capability)

    enum LogLevel {
        Log_Info,
        Log_Warning,
        Log_Error,
    };

    enum SpeedLimit {
        VisibleSpeedLimit = 0x01,
        InvisibleSpeedLimit = 0x02,
    };

    enum DeleteOption {
        DeleteTemporaryFiles = 0x00000001,
        DeleteFiles = 0x00000002,
    };
    Q_DECLARE_FLAGS(DeleteOptions, DeleteOption)
    typedef int ChangesFlags;

    Transfer(TransferGroup *parent, TransferFactory *factory, Scheduler *scheduler, const QUrl &src, const QUrl &dest, const QDomElement *e = nullptr);

    ~Transfer() override;

    /**
     * Returns the capabilities this Transfer supports
     */
    Capabilities capabilities() const
    {
        return m_capabilities;
    }

    /**
     * This functions gets called whenever a Transfer gets created. As opposed
     * to init(), this isn't a virtual function and is not meant to be used in
     * transfer plugins
     */
    void create();

    /**
     * This functions gets called whenever a Transfer is going to be deleted. As opposed
     * to deinit(), this isn't a virtual function and is not meant to be used in
     * transfer plugins
     */
    void destroy(DeleteOptions options);

    /**
     * This function is called after the creation of a Transfer
     * In transfer plugins you can put here whatever needs to be initialized
     * @note this function creates a NepomukHandler
     */
    virtual void init();

    /**
     * This function is called before the deletion of a Transfer
     * In transfer plugins you can put here whatever needs to be deinitialized
     */
    virtual void deinit(DeleteOptions options)
    {
        Q_UNUSED(options);
    }

    /**
     * Tries to repair file
     * @param file the file of a download that should be repaired,
     * if not defined all files of a download are going to be repaired
     * @return true if a repair started, false if it was not necessary
     */
    virtual bool repair(const QUrl &file = QUrl())
    {
        Q_UNUSED(file)
        return false;
    }

    const QUrl &source() const
    {
        return m_source;
    }
    const QUrl &dest() const
    {
        return m_dest;
    }

    /**
     * @returns all files of this transfer
     */
    virtual QList<QUrl> files() const
    {
        return QList<QUrl>() << m_dest;
    }

    /**
     * @returns the directory the Transfer will be stored to
     */
    virtual QUrl directory() const
    {
        return KIO::upUrl(m_dest);
    }

    /**
     * Move the download to the new destination
     * @param newDirectory is a directory where the download should be stored
     * @returns true if newDestination can be used
     */
    virtual bool setDirectory(const QUrl &newDirectory);

    // Transfer status
    KIO::filesize_t totalSize() const
    {
        return m_totalSize;
    }
    KIO::filesize_t downloadedSize() const
    {
        return m_downloadedSize;
    }
    KIO::filesize_t uploadedSize() const
    {
        return m_uploadedSize;
    }
    QString statusText() const
    {
        return m_statusText;
    }
    QString statusIconName() const
    {
        return (error().iconName.isEmpty() ? m_statusIconName : error().iconName);
    }

    static QString statusText(Job::Status status);
    static QString statusIconName(Job::Status status);

    int percent() const
    {
        return m_percent;
    }
    int downloadSpeed() const
    {
        return m_downloadSpeed;
    }
    int averageDownloadSpeed() const;
    int uploadSpeed() const
    {
        return m_uploadSpeed;
    }
    int remainingTime() const override
    {
        return KIO::calculateRemainingSeconds(totalSize(), downloadedSize(), downloadSpeed());
    }
    int elapsedTime() const override;
    bool isStalled() const override
    {
        return (status() == Job::Running && downloadSpeed() == 0);
    }
    bool isWorking() const override
    {
        return downloadSpeed() > 0;
    }

    /**
     * The mirrors that are available
     * bool if it is used, int how many parallel connections are allowed
     * to the mirror
     * @param file the file for which the availableMirrors should be get
     */
    virtual QHash<QUrl, QPair<bool, int>> availableMirrors(const QUrl &file) const;

    /**
     * Set the mirrors, int the number of parallel connections to the mirror
     * bool if the mirror should be used
     * @param file the file for which the availableMirrors should be set
     * @param mirrors the mirrors
     */
    virtual void setAvailableMirrors(const QUrl &file, const QHash<QUrl, QPair<bool, int>> &mirrors)
    {
        Q_UNUSED(file)
        Q_UNUSED(mirrors)
    }

    /**
     * Set the Transfer's UploadLimit
     * @note this is not displayed in any GUI, use setVisibleUploadLimit(int) instead
     * @param ulLimit upload Limit
     * @param limit speed limit
     */
    void setUploadLimit(int ulLimit, SpeedLimit limit);

    /**
     * Set the Transfer's UploadLimit, which are displayed in the GUI
     * @note this is not displayed in any GUI, use setVisibleDownloadLimit(int) instead
     * @param dlLimit upload Limit
     * @param limit speed limit
     */
    void setDownloadLimit(int dlLimit, SpeedLimit limit);

    /**
     * @return the UploadLimit, which is invisible in the GUI
     */
    int uploadLimit(SpeedLimit limit) const;

    /**
     * @return the DownloadLimit, which is invisible in the GUI
     */
    int downloadLimit(SpeedLimit limit) const;

    /**
     * Set the maximum share-ratio
     * @param ratio the new maximum share-ratio
     */
    void setMaximumShareRatio(double ratio);

    /**
     * @return the maximum share-ratio
     */
    double maximumShareRatio()
    {
        return m_ratio;
    }

    /**
     * Recalculate the share ratio
     */
    void checkShareRatio();

    bool isSelected() const
    {
        return m_isSelected;
    }

    /**
     * Transfer history
     */
    const QStringList log() const;

    /**
     * The owner group
     */
    TransferGroup *group() const
    {
        return (TransferGroup *)m_jobQueue;
    }

    /**
     * @return the associated TransferHandler
     */
    TransferHandler *handler();

    /**
     * @returns the TransferTreeModel that owns this group
     */
    TransferTreeModel *model();

    /**
     * @returns a pointer to the TransferFactory object
     */
    TransferFactory *factory() const
    {
        return m_factory;
    }

    /**
     * @returns a pointer to the FileModel containing all files of this download
     */
    virtual FileModel *fileModel()
    {
        return nullptr;
    }

    /**
     * @param file for which to get the verifier
     * @return Verifier that allows you to add checksums manually verify a file etc.
     */
    virtual Verifier *verifier(const QUrl &file)
    {
        Q_UNUSED(file)
        return nullptr;
    }

    /**
     * @param file for which to get the signature
     * @return Signature that allows you to add signatures and verify them
     */
    virtual Signature *signature(const QUrl &file)
    {
        Q_UNUSED(file)
        return nullptr;
    }

    /**
     * Saves this transfer to the given QDomNode
     *
     * @param element The pointer to the QDomNode where the transfer will be saved
     */
    virtual void save(const QDomElement &element);

    /**
     * Loads the transfer's info from the QDomElement
     *
     * @param element The pointer to the QDomNode where info will be loaded from
     */
    virtual void load(const QDomElement *element);

Q_SIGNALS:
    /**
     * Emitted when the capabilities of the Transfer change
     */
    void capabilitiesChanged();

public Q_SLOTS:
    /**
     * Set Transfer history
     */
    void setLog(const QString &message, Transfer::LogLevel level = Log_Info);

protected:
    /**
     * Sets the Job status to jobStatus, the status text to text and
     * the status icon to iconName.
     */
    void setStatus(Job::Status jobStatus, const QString &text = QString(), const QString &iconName = QString());

    /**
     * Sets the capabilities and automatically emits capabilitiesChanged
     */
    void setCapabilities(Capabilities capabilities);

    /**
     * Makes the TransferHandler associated with this transfer know that
     * a change in this transfer has occurred.
     *
     * @param change the TransferChange flags to be set
     * @param postEvent whether the post event is taken into account
     */
    virtual void setTransferChange(ChangesFlags change, bool postEvent = false);

    /**
     * Function used to set the SpeedLimits to the transfer
     */
    virtual void setSpeedLimits(int uploadLimit, int downloadLimit)
    {
        Q_UNUSED(uploadLimit)
        Q_UNUSED(downloadLimit)
    }

    // --- Transfer information ---
    QUrl m_source;
    QUrl m_dest;

    QStringList m_log;
    KIO::filesize_t m_totalSize;
    KIO::filesize_t m_downloadedSize;
    KIO::filesize_t m_uploadedSize;
    int m_percent;
    int m_downloadSpeed;
    int m_uploadSpeed;

    int m_uploadLimit;
    int m_downloadLimit;

    bool m_isSelected;

private:
    Capabilities m_capabilities;
    int m_visibleUploadLimit;
    int m_visibleDownloadLimit;
    int m_runningSeconds;
    double m_ratio;

    QString m_statusText;
    QString m_statusIconName;
    QElapsedTimer m_runningTime;

    TransferHandler *m_handler;
    TransferFactory *m_factory;
};

Q_DECLARE_OPERATORS_FOR_FLAGS(Transfer::Capabilities)
Q_DECLARE_OPERATORS_FOR_FLAGS(Transfer::DeleteOptions)

#endif
