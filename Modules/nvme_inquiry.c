

static const struct pr_ops nvme_pr_ops = {
        .pr_register    = nvme_pr_register,
        .pr_reserve     = nvme_pr_reserve,
        .pr_release     = nvme_pr_release,
        .pr_preempt     = nvme_pr_preempt,
        .pr_clear       = nvme_pr_clear,
};

static const struct block_device_operations nvme_fops = {
        .owner          = THIS_MODULE,
        .ioctl          = nvme_ioctl,
        .compat_ioctl   = nvme_compat_ioctl,
        .open           = nvme_open,
        .release        = nvme_release,
        .getgeo         = nvme_getgeo,
        .revalidate_disk= nvme_revalidate_disk,
        .pr_ops         = &nvme_pr_ops,
};


static int nvme_ioctl(struct block_device *bdev, fmode_t mode,
                unsigned int cmd, unsigned long arg)
{
        struct nvme_ns *ns = bdev->bd_disk->private_data;

        switch (cmd) {
        case NVME_IOCTL_ID:
                force_successful_syscall_return();
                return ns->ns_id;
        case NVME_IOCTL_ADMIN_CMD:
                return nvme_user_cmd(ns->ctrl, NULL, (void __user *)arg);
        case NVME_IOCTL_IO_CMD:
                return nvme_user_cmd(ns->ctrl, ns, (void __user *)arg);
        case NVME_IOCTL_SUBMIT_IO:
                return nvme_submit_io(ns, (void __user *)arg);
#ifdef CONFIG_BLK_DEV_NVME_SCSI
        case SG_GET_VERSION_NUM:
                return nvme_sg_get_version_num((void __user *)arg);
        case SG_IO:
                return nvme_sg_io(ns, (void __user *)arg);
#endif
        default:
                return -ENOTTY;
        }
}



int nvme_sg_io(struct nvme_ns *ns, struct sg_io_hdr __user *u_hdr)
{
        struct sg_io_hdr hdr;
        int retcode;

        if (!capable(CAP_SYS_ADMIN))
                return -EACCES;
        if (copy_from_user(&hdr, u_hdr, sizeof(hdr)))
                return -EFAULT;

        /*
         * A positive return code means a NVMe status, which has been
         * translated to sense data.
         */
        retcode = nvme_scsi_translate(ns, &hdr);
        if (retcode < 0)
                return retcode;
        if (copy_to_user(u_hdr, &hdr, sizeof(sg_io_hdr_t)) > 0)
                return -EFAULT;
        return 0;
}


static int nvme_scsi_translate(struct nvme_ns *ns, struct sg_io_hdr *hdr)
{
        u8 cmd[BLK_MAX_CDB];
        int retcode;
        unsigned int opcode;

        if (copy_from_user(cmd, hdr->cmdp, hdr->cmd_len))
                return -EFAULT;

        /*
         * Prime the hdr with good status for scsi commands that don't require
         * an nvme command for translation.
         */
        retcode = nvme_trans_status_code(hdr, NVME_SC_SUCCESS);
        if (retcode)
                return retcode;

        opcode = cmd[0];

        switch (opcode) {
        case READ_6:
        case READ_10:
        case READ_12:
        case READ_16:
                retcode = nvme_trans_io(ns, hdr, 0, cmd);
                break;
        case WRITE_6:
        case WRITE_10:
        case WRITE_12:
        case WRITE_16:
                retcode = nvme_trans_io(ns, hdr, 1, cmd);
                break;
        case INQUIRY:
                retcode = nvme_trans_inquiry(ns, hdr, cmd);
                break;
        }
        return retcode;
}

static int nvme_trans_inquiry(struct nvme_ns *ns, struct sg_io_hdr *hdr,
                                                        u8 *cmd)
{
        evpd = cmd[1] & 0x01;
        page_code = cmd[2];
        alloc_len = get_unaligned_be16(&cmd[3]);

        if (evpd == 0) {
                if (page_code == INQ_STANDARD_INQUIRY_PAGE) {
                        res = nvme_trans_standard_inquiry_page(ns, hdr,
                                                inq_response, alloc_len);
                } else {
                        res = nvme_trans_completion(hdr,
                                                SAM_STAT_CHECK_CONDITION,
                                                ILLEGAL_REQUEST,
                                                SCSI_ASC_INVALID_CDB,
                                                SCSI_ASCQ_CAUSE_NOT_REPORTABLE);
                }
        } else {
        }

        kfree(inq_response);
 out_mem:
        return res;
}



/* INQUIRY Helper Functions */

static int nvme_trans_standard_inquiry_page(struct nvme_ns *ns,
                                        struct sg_io_hdr *hdr, u8 *inq_response,
                                        int alloc_len)
{

        /* nvme ns identify - use DPS value for PROTECT field */
        nvme_sc = nvme_identify_ns(ctrl, ns->ns_id, &id_ns);
        res = nvme_trans_status_code(hdr, nvme_sc);

        xfer_len = min(alloc_len, STANDARD_INQUIRY_LENGTH);
        return nvme_trans_copy_to_user(hdr, inq_response, xfer_len);
}


int nvme_identify_ns(struct nvme_ctrl *dev, unsigned nsid,
                struct nvme_id_ns **id)
{
        struct nvme_command c = { };
        int error;

        /* gcc-4.4.4 (at least) has issues with initializers and anon unions */
        c.identify.opcode = nvme_admin_identify,
        c.identify.nsid = cpu_to_le32(nsid),

        *id = kmalloc(sizeof(struct nvme_id_ns), GFP_KERNEL);
        if (!*id)
                return -ENOMEM;

        error = nvme_submit_sync_cmd(dev->admin_q, &c, *id,
                        sizeof(struct nvme_id_ns));
        if (error)
                kfree(*id);
        return error;
}


/*
 * Returns 0 on success.  If the result is negative, it's a Linux error code;
 * if the result is positive, it's an NVM Express status code
 */
int __nvme_submit_sync_cmd(struct request_queue *q, struct nvme_command *cmd,
                struct nvme_completion *cqe, void *buffer, unsigned bufflen,
                unsigned timeout, int qid, int at_head, int flags)
{
        struct request *req;
        int ret;

        req = nvme_alloc_request(q, cmd, flags, qid);
        if (IS_ERR(req))
                return PTR_ERR(req);

        req->timeout = timeout ? timeout : ADMIN_TIMEOUT;
        req->special = cqe;

        if (buffer && bufflen) {
                ret = blk_rq_map_kern(q, req, buffer, bufflen, GFP_KERNEL);
                if (ret)
                        goto out;
        }

        blk_execute_rq(req->q, NULL, req, at_head);
        ret = req->errors;
 out:
        blk_mq_free_request(req);
        return ret;
}
EXPORT_SYMBOL_GPL(__nvme_submit_sync_cmd);

int nvme_submit_sync_cmd(struct request_queue *q, struct nvme_command *cmd,
                void *buffer, unsigned bufflen)
{
        return __nvme_submit_sync_cmd(q, cmd, NULL, buffer, bufflen, 0,
                        NVME_QID_ANY, 0, 0);
}
EXPORT_SYMBOL_GPL(nvme_submit_sync_cmd);


/* Copy data to userspace memory */

static int nvme_trans_copy_to_user(struct sg_io_hdr *hdr, void *from,
                                                                unsigned long n)
{
        int i;
        void *index = from;
        size_t remaining = n;
        size_t xfer_len;

        if (hdr->iovec_count > 0) {
                struct sg_iovec sgl;

                for (i = 0; i < hdr->iovec_count; i++) {
                        if (copy_from_user(&sgl, hdr->dxferp +
                                                i * sizeof(struct sg_iovec),
                                                sizeof(struct sg_iovec)))
                                return -EFAULT;
                        xfer_len = min(remaining, sgl.iov_len);
                        if (copy_to_user(sgl.iov_base, index, xfer_len))
                                return -EFAULT;

                        index += xfer_len;
                        remaining -= xfer_len;
                        if (remaining == 0)
                                break;
                }
                return 0;
        }

        if (copy_to_user(hdr->dxferp, from, n))
                return -EFAULT;
        return 0;
}

