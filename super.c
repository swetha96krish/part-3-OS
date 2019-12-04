#include <linux/init.h>
#include <linux/module.h>
#include <linux/fs.h>
#include <linux/uaccess.h>
#define S2FS_MAGIC 0x19920342

#define TMPSIZE 20

static int s2fs_fill_super (struct super_block *sb, void *data, int silent);
static struct dentry *s2fs_get_super(struct file_system_type *fst,
                int flags, const char *devname, void *data)
{
        printk(KERN_NOTICE "mounting");
        return mount_nodev(fst, flags, data, s2fs_fill_super);
}

static struct file_system_type s2fs_type = {
        .owner          = THIS_MODULE,
        .name           = "s2fs",
        .mount          = s2fs_get_super,
        .kill_sb        = kill_litter_super,
};

static ssize_t s2fs_read_file(struct file *filp, char *buf,
		size_t count, loff_t *offset)
{
	char tmp[TMPSIZE] = "Hello World!\n";
	if(*offset >= strlen(tmp))
	return 0;
	if (copy_to_user(buf, tmp, strlen(tmp)))
	return -EFAULT;
	*offset += strlen(tmp);
	return 14;
}
static int s2fs_open(struct inode *inode, struct file *filp)
{
	return 0;
}

static ssize_t s2fs_write_file(struct file *filp, const char *buf,
		size_t count, loff_t *offset)
{
	return 0;
}

static struct file_operations s2fs_fops = {
	.open	= s2fs_open,
	.read 	= s2fs_read_file,
	.write  = s2fs_write_file,
};
static struct inode *s2fs_make_inode(struct super_block *sb,int mode)
{
        struct inode *inode = new_inode(sb);
        if(!inode){
                return NULL;
        }
        inode->i_mode = mode;
        inode->i_atime = inode->i_mtime = inode->i_ctime = current_time(inode);
        inode->i_ino = get_next_ino();
        return inode;
}

static struct dentry *s2fs_create_file(struct super_block *sb,struct dentry *dir,const char *name)
{
        struct dentry *dentry;
        struct inode *inode;
        dentry = d_alloc_name(dir,name);
        if(!dentry)
                return 0;
        inode = s2fs_make_inode(sb,S_IFREG|0644);
	inode->i_fop = &s2fs_fops;
        if(!inode)
                dput(dentry);
        d_add(dentry,inode);
        return dentry;
}

static struct dentry *s2fs_create_dir(struct super_block *sb,struct dentry *parent,const char *dir_name)
{
        struct dentry *dentry = d_alloc_name(parent,dir_name);
        struct  inode *inode;

        if(! dentry)
                return 0;

        inode = s2fs_make_inode(sb, S_IFDIR|0755);

        if(! inode)
                dput(dentry);
	inode->i_op = &simple_dir_inode_operations;
	inode->i_fop=&simple_dir_operations;
        d_add(dentry, inode);
        return dentry;
}

static int s2fs_fill_super (struct super_block *sb, void *data, int silent)
{
	struct inode *root;
        struct dentry *root_dentry;
	struct dentry *dir;
/*
 * Basic parameters.
 */
        sb->s_blocksize = VMACACHE_SIZE;
        sb->s_blocksize_bits = VMACACHE_SIZE;
        sb->s_magic = S2FS_MAGIC;
        //sb->s_op = &s2fs_s_ops;
/*
 * We need to conjure up an inode to represent the root directory
 * of this filesystem.  Its operations all come from libfs, so we
 * don't have to mess with actually *doing* things inside this
 * directory.
 */
        root = s2fs_make_inode (sb, S_IFDIR | 0755);
        inode_init_owner(root, NULL, S_IFDIR | 0755);
        if (! root)
                goto out;
      root->i_op = &simple_dir_inode_operations;
      root->i_fop = &simple_dir_operations;
/*
 * Get a dentry to represent the directory in core.
 */
        set_nlink(root, 2);
        root_dentry = d_make_root(root);
        if (! root_dentry)
                goto out_iput;

	dir = s2fs_create_dir(sb,root_dentry,"foo");
	if(dir)
	     s2fs_create_file(sb,dir,"bar");
	sb->s_root = root_dentry;
	return 0;
 out_iput:
        iput(root);
  out:
        return -ENOMEM;

}

static int __init s2fs_init(void)
  {
      return register_filesystem(&s2fs_type);
      return 0;
  }

  static void __exit s2fs_fini(void)
  {
    printk(KERN_NOTICE "unmounting");
    unregister_filesystem(&s2fs_type);
  }

  module_init(s2fs_init);
  module_exit(s2fs_fini);

  MODULE_LICENSE("GPL");
