CREATE TABLE tags (
    instance int default 0,
    rfid varchar(24),
    item_supplier varchar(24),
    item_id varchar(24),
    ploc varchar(32),
    ploc_at double,
    ploc_dev varchar(24),
    tloc varchar(32),
    tloc_at double,
    tloc_dev varchar(24),
    data varchar(64),
    new_data varchar(64),
    primary key (instance, rfid));
CREATE INDEX tags_item_id on tags (instance, item_id);
CREATE INDEX tags_ploc on tags (instance, ploc, ploc_at);
CREATE INDEX tags_ploc_dev on tags(instance, ploc_dev, ploc_at);
CREATE INDEX tags_tloc on tags(instance, tloc, tloc_at);

CREATE TABLE items (
    instance int default 0,
    item_supplier varchar(24),
    item_id varchar(24),
    product_id varchar(24),
    json blob,
    last_seen double,
    primary key (instance, item_supplier, item_id));
CREATE INDEX items_product ON items (instance, product_id);

CREATE TABLE items_idx (
    instance int default 0,
    item_supplier varchar(24),
    item_id varchar(24),
    word varchar(32));
CREATE INDEX items_idx_id ON items_idx (instance, item_supplier, item_id);
CREATE INDEX items_idx_word ON items_idx (instance, word);

CREATE TABLE devices (
    id varchar(24),
    last_at double,
    version varchar(64),
    update_path varchar(256),
    nickname varchar(24),
    branch varchar(12),
    last_update_path varchar(256),
    instance int
    );

CREATE TABLE history (dev varchar(32),
    instance int default 0,
    at double,
    rfid varchar(32),
    action varchar(80),
    type varchar(32),
    shelf varchar(32)
    );
CREATE INDEX history_at on history(instance, at);
CREATE INDEX history_dev on history(instance, dev, at);
CREATE INDEX history_rfid on history(instance, rfid, at);

CREATE TABLE location_tree (
    instance int default 0,
    loc varchar(32),
    count int,
    PRIMARY KEY (instance, loc));
