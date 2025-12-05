CREATE TABLE IF NOT EXISTS urls (
    id BIGSERIAL PRIMARY KEY,
    short_code TEXT NOT NULL UNIQUE,
    original_url TEXT NOT NULL,
    created_at TIMESTAMPTZ NOT NULL DEFAULT NOW()
);
CREATE INDEX IF NOT EXISTS idx_urls_short_code ON urls (short_code);

CREATE TABLE IF NOT EXISTS clicks (
    id BIGSERIAL PRIMARY KEY,
    
    url_id BIGINT NOT NULL REFERENCES urls(id) ON DELETE CASCADE,
    
    ts TIMESTAMPTZ NOT NULL DEFAULT NOW(),
    ip_address INET NOT NULL,
    user_agent TEXT,
    
    referer TEXT, 
    language TEXT,
    http_method TEXT,
    platform TEXT,
    is_mobile BOOLEAN DEFAULT FALSE,
    country_code TEXT,
    trace_id TEXT 
);


CREATE INDEX IF NOT EXISTS idx_clicks_url_id ON clicks (url_id);
CREATE INDEX IF NOT EXISTS idx_clicks_ts ON clicks (ts);