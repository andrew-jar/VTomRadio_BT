# WWW workflow

This folder is the source of truth for editable web assets.

## Edit flow

1. Edit files in `www_work/` (for example: `bt.html`, `player.html`, `options.html`, `script.js`).
2. Run:

```bash
python www_work/gzip_to_data_www.py
```

3. The script syncs each file to `data/www/` in two forms:
   - plain file (for easier debugging)
   - `.gz` file (for runtime serving/compression)

4. Upload filesystem to device (`Upload Filesystem Image` / uploadfs).

## Notes

- Do not edit runtime files in `data/www/` manually unless you really need an emergency hotfix.
- If you edit runtime directly, copy the change back to `www_work/` to keep sources consistent.
