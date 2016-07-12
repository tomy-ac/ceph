=================
 Ceph Filesystem
=================

The :term:`Ceph Filesystem` (Ceph FS) is a POSIX-compliant filesystem that uses
a Ceph Storage Cluster to store its data. The Ceph filesystem uses the same Ceph
Storage Cluster system as Ceph Block Devices, Ceph Object Storage with its S3
and Swift APIs, or native bindings (librados).

.. note:: If you are evaluating CephFS for the first time, please review
          the best practices for deployment: :doc:`/cephfs/best-practices`

.. ditaa::
            +-----------------------+  +------------------------+
            |                       |  |      CephFS FUSE       |
            |                       |  +------------------------+
            |                       |
            |                       |  +------------------------+
            |  CephFS Kernel Object |  |     CephFS Library     |
            |                       |  +------------------------+
            |                       |
            |                       |  +------------------------+
            |                       |  |        librados        |
            +-----------------------+  +------------------------+

            +---------------+ +---------------+ +---------------+
            |      OSDs     | |      MDSs     | |    Monitors   |
            +---------------+ +---------------+ +---------------+


Using the Ceph Filesystem requires at least one :term:`Ceph Metadata Server` in
your Ceph Storage Cluster.



.. raw:: html

	<style type="text/css">div.body h3{margin:5px 0px 0px 0px;}</style>
	<table cellpadding="10"><colgroup><col width="33%"><col width="33%"><col width="33%"></colgroup><tbody valign="top"><tr><td><h3>Step 1: Metadata Server</h3>

To run the Ceph Filesystem, you must have a running Ceph Storage Cluster with at
least one :term:`Ceph Metadata Server` running.


.. toctree:: 
	:maxdepth: 1

	Add/Remove MDS <../../rados/deployment/ceph-deploy-mds>
	MDS failover and standby configuration <standby>
	MDS Configuration Settings <mds-config-ref>
	Journaler Configuration <journaler>
	Manpage ceph-mds <../../man/8/ceph-mds>

.. raw:: html 

	</td><td><h3>Step 2: Mount CephFS</h3>

Once you have a healthy Ceph Storage Cluster with at least
one Ceph Metadata Server, you may create and mount your Ceph Filesystem.
Ensure that you client has network connectivity and the proper
authentication keyring.

.. toctree:: 
	:maxdepth: 1

	Create CephFS <createfs>
	Mount CephFS <kernel>
	Mount CephFS as FUSE <fuse>
	Mount CephFS in fstab <fstab>
	Manpage cephfs <../../man/8/cephfs>
	Manpage ceph-fuse <../../man/8/ceph-fuse>
	Manpage mount.ceph <../../man/8/mount.ceph>


.. raw:: html 

	</td><td><h3>Additional Details</h3>

.. toctree:: 
	:maxdepth: 1

    Deployment best practices <best-practices>
    Administrative commands <administration>
	POSIX compatibility <posix>
	Experimental Features <experimental-features>
        CephFS Quotas <quota>
	Using Ceph with Hadoop <hadoop>
	libcephfs <../../api/libcephfs-java/>
	cephfs-journal-tool <cephfs-journal-tool>
	File layouts <file-layouts>
	Client eviction <eviction>
	Handling full filesystems <full>
	Troubleshooting <troubleshooting>
	Disaster recovery <disaster-recovery>
	Client authentication <client-auth>
	Upgrading old filesystems <upgrading>

.. raw:: html

	</td></tr></tbody></table>
